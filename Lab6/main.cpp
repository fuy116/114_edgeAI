#include <iostream>
#include <thread>

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/dnn.hpp>
#include "RtpVideoStreamer.hpp"
#include "curl/curl.h"

class ROI {
public:
    int startX, startY, width, height;
    ROI(int s_x, int s_y, int w, int h):startX(s_x), startY(s_y), width(w), height(h){}

    cv::Rect getRect()
    {
        return cv::Rect(startX, startY, width, height);    
    }
};

struct DoorMarker {
    int X, Y, radius, threshold;
    DoorMarker(int x, int y, int r, int th):X(x), Y(y), radius(r), threshold(th){}
};

struct elevator_data {
    int elevator_id, people_inside_count, people_outside_count;
    std::string door_status;
    std::string timestamp;
    elevator_data(int id):elevator_id(id), people_inside_count(0), people_outside_count(0),
                            door_status("Close"), timestamp("2025/01/01"){}
};

static std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
    return std::string(buffer);
}

elevator_data ele_data(1);

class Client {
private:
    cv::VideoCapture cap;
    cv::dnn::Net net;
    cv::VideoWriter writer;
    cv::VideoWriter output_video;
    // If socket functionality is needed, add related objects here
    // Socket client_socket;
    
    // Magic number (accroding to the video position)
    DoorMarker door_marker{260, 130, 5, 10};
    ROI outside_roi{180, 100, 730 - 180, 400 - 100};
    ROI elevator_roi{185, 80, 350 - 185, 390 - 80};
    ROI door_roi{door_marker.X, door_marker.Y, door_marker.radius, door_marker.radius};
        
    int current_inside = 0;
    int current_outside = 0;

    // send to db
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = nullptr;
    std::string json_data;
    nlohmann::json data = {
        {"elevator_id", "1"},
        {"timestamp", "2025/1/1"},
        {"people_inside_count", "0"},
        {"people_outside_count", "0"},
    };

private:
    bool is_elevator_open(cv::Mat &frame)
    {
        cv::Mat door_frame = frame(door_roi.getRect());
        cv::Scalar door_mean_color = cv::mean(door_frame);
        
        if (door_mean_color[1] - door_mean_color[0] > door_marker.threshold) {
            ele_data.door_status = "Open";
            return true;
        }

        ele_data.door_status = "Close";
        return false;
    }


public:
    Client() 
    {
        curl_global_init(CURL_GLOBAL_ALL); 
        curl = curl_easy_init();
        headers = curl_slist_append(headers, "Content-Type:application/json");
        curl_easy_setopt(curl, CURLOPT_URL, "https://urufscarpvif.ap-northeast-1.clawcloudrun.com/mongo/");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Capture
        std::string pipeline = "libcamerasrc ! video/x-raw,width=960,height=540,framerate=30/1,format=RGB ! videoconvert ! appsink";
         
        //cap = cv::VideoCapture (pipeline, cv::CAP_GSTREAMER);
        //cap = cv::VideoCapture("../original_video/full.mp4");
        cap = cv::VideoCapture("../original_video/IMG_9018-Resized.mp4");

        if (!cap.isOpened()) {
            std::cerr << "Can not open camera" << std::endl;
            return;
        }

        //writer.open("appsrc ! videoconvert ! x264enc tune=zerolatency bitrate=1000 key-int-max=30 speed-preset=ultrafast ! video/x-h264,profile=constrained-baseline ! h264parse ! queue ! rtph264pay pt= 96 mtu= 1200 ! udpsink host=127.0.0.1 port=6000", cv::CAP_GSTREAMER, 0, 30, cv::Size(960, 540);

        // Load model
        net = cv::dnn::readNetFromCaffe("../MobileNet-SSD/deploy.prototxt", "../MobileNet-SSD/mobilenet_iter_73000.caffemodel");

        // Setup output video - mp4
        output_video = cv::VideoWriter(
            "output_video.mp4",
            cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30,
            cv::Size(static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH)),
                    static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT))),
            true
        );

        // Client socket instance
        // client_socket.conn();

        // Confirm whether the model has been loaded successfully
        if (net.empty()) {
            std::cout << "Fail to Load Model" << std::endl;
            return;
        } else {
            std::cout << "Success to Load Model" << std::endl;
        }
    }
    void draw_info(cv::Mat &frame)
    {
        //cv::rectangle(frame, cv::Point(door_roi.startX, door_roi.startY),      
        //  cv::Point(door_roi.startX + door_roi.width, door_roi.startY + door_roi.height),
        //  cv::Scalar(0, 255, 0),  
        //  2);                     

        cv::rectangle(frame, cv::Point(outside_roi.startX, outside_roi.startY),      
          cv::Point(outside_roi.startX + outside_roi.width, outside_roi.startY + outside_roi.height),
          cv::Scalar(0, 255, 0),  
          2);                     

        cv::rectangle(frame, cv::Point(outside_roi.startX, outside_roi.startY),      
          cv::Point(elevator_roi.startX + elevator_roi.width, elevator_roi.startY + elevator_roi.height),
          cv::Scalar(255, 0, 0),  
          2);                     

        cv::putText(frame,
            ele_data.door_status,
            cv::Point(10, 30),       
            cv::FONT_HERSHEY_SIMPLEX, 
            1.0,                      
            cv::Scalar(0, 0, 255),
        2);                           

        cv::putText(frame,
            "Inside Count:" + std::to_string(ele_data.people_inside_count),
            cv::Point(10, 60),       
            cv::FONT_HERSHEY_SIMPLEX, 
            1.0,                      
            cv::Scalar(0, 0, 255),
        2);                           

        cv::putText(frame,
            "Outside Count:" + std::to_string(ele_data.people_outside_count),
            cv::Point(10, 90),       
            cv::FONT_HERSHEY_SIMPLEX, 
            1.0,                      
            cv::Scalar(0, 0, 255),
        2);                           
    }

    void update_db()
    {
        data["timestamp"] = ele_data.timestamp;
        data["people_inside_count"] = ele_data.people_inside_count;
        data["people_outside_count"] = ele_data.people_outside_count;
        json_data = data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str()); 

        if (curl) {
            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
                std::cerr << "curl_easy_perform() post return " << curl_easy_strerror(res) << std::endl; 
        }
        
        return;
    }
    void broadcast()
    {
         
    }
    
    void predict()
    {
        const int SKIP_FRAMES = 0;
        int frame_count = 0;
        int outside_count_when_open = 0;

        bool ret;

        ROI roi(0, 0, 0, 0);
        cv::Mat show_frame, predict_frame, roi_frame, blob, detections;
        while (true){
            cap >> show_frame;
            if (show_frame.empty()) break;

            predict_frame = show_frame.clone();

            draw_info(show_frame);


            //writer.write(show_frame);

            frame_count++;
           
            if (SKIP_FRAMES != 0 && frame_count % SKIP_FRAMES != 0)
                continue;
            
            frame_count = 0;

            bool elevator_open = is_elevator_open(predict_frame);

            if (elevator_open) {
                roi = elevator_roi;
                roi_frame = predict_frame(elevator_roi.getRect());
            }
            else {
                roi = outside_roi;
                roi_frame = predict_frame(outside_roi.getRect());
            }

            blob = cv::dnn::blobFromImage(roi_frame, 0.007843, cv::Size(300, 300), 127.5);
            this->net.setInput(blob);
            detections = this->net.forward();

            int people_count = 0;
            int num_detections = detections.size[2];

            for (int i = 0; i < num_detections; i++) {

                float* detection = (float*)detections.ptr<float>(0) + i * 7;
                float confidence = detection[2];

                if (confidence > 0.9) {
                    int class_id = detection[1];

                    if (class_id == 15) {
                        people_count++;
                        
                        int x_min = detection[3] * roi_frame.cols + roi.startX;
                        int y_min = detection[4] * roi_frame.rows + roi.startY;
                        int x_max = detection[5] * roi_frame.cols + roi.startX;
                        int y_max = detection[6] * roi_frame.rows + roi.startY;

                        cv::rectangle(show_frame, cv::Point(x_min, y_min),      
                          cv::Point(x_max, y_max),
                          cv::Scalar(0, 255, 0),  
                          2);                     
                    }
                }
            }

            if (elevator_open) {
                ele_data.people_inside_count = std::max(ele_data.people_inside_count, people_count);
                ele_data.people_outside_count = outside_count_when_open - ele_data.people_inside_count;
            }
            else { 
                ele_data.people_outside_count = people_count;
                outside_count_when_open = ele_data.people_outside_count;
            }
            cv::imshow("Video", show_frame);
            if (cv::waitKey(1) == 'q')
                 break;

            output_video.write(show_frame);
            ele_data.timestamp = get_current_timestamp();
            update_db();
        }
    }

    ~Client() 
    {
        // Release resources
        cap.release();
        writer.release();
        output_video.release();
        cv::destroyAllWindows();
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

};

int main()
{
    Client model = Client(); 
    model.predict();
    
    //try {
    //    RtpVideoStreamer streamer("127.0.0.1", 6000);
    //    streamer.start();

    //    std::cout << "Press Enter to exit...\n";
    //    std::cin.get();
    //} catch (const std::exception& e) {
    //    std::cerr << "Exception: " << e.what() << std::endl;
    //}
    return 0;
}
