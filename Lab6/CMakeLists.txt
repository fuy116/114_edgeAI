cmake_minimum_required(VERSION 3.10)
project(ElevatorPredictProject)

# Find package
find_package(OpenCV REQUIRED)
find_package(LibDataChannel REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

message(STATUS "OpenCV found: version ${OpenCV_VERSION}")
message(STATUS "LibDataChannel found: version ${LibDataChannel_VERSION}")
message(STATUS "nlohmann_json found: version ${nlohmann_json_VERSION}")
message(STATUS "CURL found: version ${CURL_VERSION}")
message(STATUS "libwebsockets found: version ${libwebsockets_VERSION}")

# Add source files
add_executable(app main.cpp RtpVideoStreamer.cpp)
#add_executable(app test.cpp)

# Include headers
target_include_directories(app PRIVATE ${OpenCV_INCLUDE_DIRS})
#target_include_directories(app PRIVATE ${libwebsockets_INCLUDE_DIRS})

# Link libraries
target_link_libraries(app PRIVATE 
    ${OpenCV_LIBS} 
    LibDataChannel::LibDataChannel 
    Threads::Threads 
    nlohmann_json::nlohmann_json 
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system 
    Boost::thread
)
