import cv2
import numpy as np

def calculate_overlap(box, elevator_area):
    # box: [x_min, y_min, x_max, y_max]
    # 計算重疊區域

    x_left = max(box[0], elevator_area['x_min'])
    y_top = max(box[1], elevator_area['y_min'])
    x_right = min(box[2], elevator_area['x_max'])
    y_bottom = min(box[3], elevator_area['y_max'])
    
    if x_right < x_left or y_bottom < y_top:
        return 0.0
        
    intersection_area = (x_right - x_left) * (y_bottom - y_top)
    box_area = (box[2] - box[0]) * (box[3] - box[1])
    
    overlap_ratio = intersection_area / box_area
    return overlap_ratio

def main():
    # load video
    cap = cv2.VideoCapture("original_video/full.mp4")
    # load model
    net = cv2.dnn.readNetFromCaffe("MobileNet-SSD/deploy.prototxt", "MobileNet-SSD/mobilenet_iter_73000.caffemodel")
    # set box (ROI)
    roi_startX, roi_startY, roi_endX, roi_endY = 180, 100, 730, 400
    elevator_startX, elevator_endX, elevator_startY, elevator_endY = 185, 350, 80, 390
    # output video - mp4
    output_video = cv2.VideoWriter('output_video.mp4', cv2.VideoWriter_fourcc(*'MP4V'), 30, (int(cap.get(3)), int(cap.get(4))), True)

    # Confirm whether the model has been introduced
    if net.empty():
        print("Fail to Load Model")
        return
    else:
        print("Success to Load Load")

    door_marker_x, door_marker_y = 260, 130
    door_marker_radius = 5

    door_marker_color_threshold = 10

    max_inside = 0
    ppl_outside = 0 
    
    while True:
        # Read Frame
        ret, frame = cap.read()
        if not ret:
            break

        door_roi = frame[door_marker_y - door_marker_radius: door_marker_y + door_marker_radius, door_marker_x - door_marker_radius: door_marker_x + door_marker_radius]

        elevator_area = {
            'x_min': elevator_startX,
            'x_max': elevator_endX,
            'y_min': elevator_startY,
            'y_max': elevator_endY
        }

        # Draw rectangle around detected object inside the ROI
        cv2.rectangle(frame, (elevator_area['x_min'], elevator_area['y_min']), (elevator_area['x_max'], elevator_area['y_max']), (0, 255, 0), 2)

        cv2.putText(frame, "Elevator", (elevator_area['x_min'], elevator_area['y_min'] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

        door_mean_color = np.mean(door_roi, axis=(0, 1))

        if (door_mean_color[1] - door_mean_color[0] > door_marker_color_threshold):
            elevator_doors_open = True
        else:
            elevator_doors_open = False
        
        door_status = "Open" if elevator_doors_open else "Close"

        cv2.putText(frame, f"Door Status: {door_status}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        # Adjust ROI (Crop the frame to the region of interest)
        if elevator_doors_open:
            roi = frame[elevator_startY:elevator_endY, elevator_startX: elevator_endX]
        else:
            roi = frame[roi_startY:roi_endY, roi_startX:roi_endX]

        # Detection object
        blob = cv2.dnn.blobFromImage(roi, 0.007843, (300, 300), 127.5)
        net.setInput(blob)
        detections = net.forward()

        # Draw rectangle for ROI (Region of Interest)
        cv2.rectangle(frame, (roi_startX, roi_startY), (roi_endX, roi_endY), (255, 0, 0), 2)

        people_count = 0  # Initialize people counter for the ROI

        # print("偵測框總數:", detections.shape[2])
        # print("第一個框資訊:", detections[0, 0, 0])
        overlap_ratio = 0.0

        ppl_count = 0

        for i in range(detections.shape[2]):

            confidence = detections[0, 0, i, 2]

            if confidence > 0.7: 
                class_id = int(detections[0, 0, i, 1])

                if class_id == 15:  # class_id for 'person'
                    people_count += 1

                    # Get bounding box coordinates relative to ROI
                    x_min = int(detections[0, 0, i, 3] * roi.shape[1])
                    y_min = int(detections[0, 0, i, 4] * roi.shape[0])
                    x_max = int(detections[0, 0, i, 5] * roi.shape[1])
                    y_max = int(detections[0, 0, i, 6] * roi.shape[0])

                    # Convert coordinates back to the original frame scale
                    x_min += roi_startX
                    y_min += roi_startY
                    x_max += roi_startX
                    y_max += roi_startY
                    
                    box = [x_min, y_min, x_max, y_max]

                    overlap_ratio = calculate_overlap(box=box, elevator_area=elevator_area)
                    # if (elevator_doors_oepn == True and overlap_ratio > 0.9):
                    ppl_count += 1

                    # Draw rectangle around detected object inside the ROI
                    cv2.rectangle(frame, (x_min, y_min), (x_max, y_max), (0, 255, 0), 2)

                    label = f"{i}"

                    cv2.putText(frame, label, (x_min, y_min - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
        
        if (elevator_doors_open == True):
            max_inside = max(max_inside, ppl_count)
            cv2.putText(frame, f"People in outside: {ppl_outside - max_inside}", (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
        else:
            ppl_outside = ppl_count
            cv2.putText(frame, f"People in outside: {ppl_outside}", (10, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        #cv2.putText(frame, f"Overlap Ratio: {overlap_ratio}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        cv2.putText(frame, f"People in elevator: {max_inside}", (10, 60), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)


        output_video.write(frame)

    # Release resources
    cap.release()
    output_video.release()
    print("Success video output")

main()