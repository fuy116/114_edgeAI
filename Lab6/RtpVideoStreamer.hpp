#pragma once

#include "rtc/rtc.hpp"
#include <nlohmann/json.hpp>

#include <string>
#include <memory>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "curl/curl.h"
typedef int SOCKET;
#endif

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class RtpVideoStreamer {
public:
    RtpVideoStreamer(const std::string& ip, uint16_t port, rtc::SSRC ssrc = 42);
    ~RtpVideoStreamer();

    void start();

private:
    void setupSocket();
    void setupPeerConnection();
    void setupWebSocket();
    void waitForAnswer();
    void getRemoteCandidate();
    void sendSDP(const std::string ip, const std::string sdp);
    void receiveLoop();

    std::string ip_;
    uint16_t port_;
    rtc::SSRC ssrc_;
    SOCKET sock_;
    std::shared_ptr<rtc::PeerConnection> pc_;
    std::shared_ptr<rtc::Track> track_;

    std::unique_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws_;
    std::thread receiveThread_;
    std::atomic<bool> running_;
    
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = nullptr;
};
