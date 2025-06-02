#include "RtpVideoStreamer.hpp"

#include <iostream>
#include <stdexcept>
#include <unistd.h>

RtpVideoStreamer::RtpVideoStreamer(const std::string& ip, uint16_t port, rtc::SSRC ssrc)
    : ip_(ip), port_(port), ssrc_(ssrc), running_(false) {
    rtc::InitLogger(rtc::LogLevel::Debug);
    setupSocket();
    setupPeerConnection();
    setupWebSocket();
}

RtpVideoStreamer::~RtpVideoStreamer() {
    running_ = false;
    if (receiveThread_.joinable()) {
        receiveThread_.join();
    }
    close(sock_);
    ws_->close(websocket::close_code::normal);
}

void RtpVideoStreamer::start() {
    pc_->setLocalDescription();
    //waitForAnswer();
    receiveThread_ = std::thread(&RtpVideoStreamer::receiveLoop, this);
}

void RtpVideoStreamer::setupSocket() {
    sock_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("Failed to create UDP socket");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    addr.sin_port = htons(port_);

    if (bind(sock_, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("Failed to bind UDP socket");
    }

    int rcvBufSize = 212992;
    setsockopt(sock_, SOL_SOCKET, SO_RCVBUF, reinterpret_cast<const char*>(&rcvBufSize),
               sizeof(rcvBufSize));
}

void RtpVideoStreamer::setupPeerConnection() {

    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    pc_ = std::make_shared<rtc::PeerConnection>(config);
    pc_->onStateChange([](rtc::PeerConnection::State state) {
        std::cout << "PeerConnection State: " << state << std::endl;
    });

    pc_->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
        std::cout << "Gathering State: " << state << std::endl;
        if (state == rtc::PeerConnection::GatheringState::Complete) {
            auto description = pc_->localDescription();
            nlohmann::json message = {
                {"type", "offer"},
                {"sdp", {
                    {"type", description->typeString()},
                    {"sdp", std::string(description.value())}
                }}
            };
            std::cout << "Local SDP: \n" << message.dump() << std::endl;
            sendSDP("https://pqvhljxqvokh.ap-northeast-1.clawcloudrun.com/ws", message.dump());
        }
    });

    rtc::Description::Video media("video", rtc::Description::Direction::SendOnly);
    media.addH264Codec(96); // Payload type must match the incoming RTP stream
    media.addSSRC(ssrc_, "video-send");
    track_ = pc_->addTrack(media);
}
void RtpVideoStreamer::setupWebSocket()
{
try {
    net::io_context ioc;

    // TLS context
    ssl::context ctx{ssl::context::tlsv12_client};
    ctx.set_default_verify_paths();

    std::string const host = "pqvhljxqvokh.ap-northeast-1.clawcloudrun.com";
    std::string const port = "443";
    std::string const target = "/ws";

    // DNS
    tcp::resolver resolver{ioc};
    auto const results = resolver.resolve(host, port);

    // SSL stream
    beast::ssl_stream<tcp::socket> ssl_stream{ioc, ctx};
    // SNI (Server Name Indication) ??
    if(!SSL_set_tlsext_host_name(ssl_stream.native_handle(), host.c_str()))
        throw beast::system_error(
                beast::error_code(static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()),
                "Failed to set SNI Hostname");

    // TCP
    net::connect(ssl_stream.next_layer(), results.begin(), results.end());

    // SSL
    ssl_stream.handshake(ssl::stream_base::client);

    // WebSocket stream (SSL stream)
    ws_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(std::move(ssl_stream));
    ws_->set_option(websocket::stream_base::decorator(
                [](websocket::request_type& req) {
                req.set(http::field::user_agent, "Boost.Beast Client");
                // req.set(http::field::authorization, "Bearer token");
                }));

    std::cout << "WebSocket Connecting to " << host << target << "...\n";

    // WebSocket
    ws_->handshake(host, target);

    std::cout << "WebSocket Connected!\n";

    } 
catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void RtpVideoStreamer::waitForAnswer() 
{
    std::cout << "Please paste the remote SDP answer:\n";
    std::string sdp;
    std::getline(std::cin, sdp);

    auto j = nlohmann::json::parse(sdp);
    rtc::Description answer(j["sdp"].get<std::string>(), j["type"].get<std::string>());
    pc_->setRemoteDescription(answer);
}

void RtpVideoStreamer::getRemoteCandidate()
{
    beast::flat_buffer candidate_buffer;
    ws_->read(candidate_buffer);
    std::string msg = beast::buffers_to_string(candidate_buffer.data());
    std::cout << "Remote candidate: " << msg << std::endl;
    auto j = nlohmann::json::parse(msg);
    rtc::Candidate candidate(j["candidate"]["candidate"].get<std::string>(), j["candidate"]["sdpMid"].get<std::string>());
    pc_->addRemoteCandidate(candidate);
    std::cout << "RemoteCandidate set successfully!" << std::endl;
}

void RtpVideoStreamer::sendSDP(const std::string ip, const std::string sdp)
{
try {
    std::string json_msg = R"({"type": "join", "role": "broadcaster"})";
    ws_->write(net::buffer(json_msg));

    ws_->write(net::buffer(sdp));

    beast::flat_buffer buffer;
    ws_->read(buffer);

    std::string sdp = beast::buffers_to_string(buffer.data());
    std::cout << "Remote Answer: " << sdp << std::endl;

    auto j = nlohmann::json::parse(sdp);
    rtc::Description answer(j["sdp"]["sdp"].get<std::string>(), j["sdp"]["type"].get<std::string>());
    pc_->setRemoteDescription(answer);
    std::cout << "RemoteDescription set successfully!" << std::endl;

    getRemoteCandidate();
    getRemoteCandidate();

    //beast::flat_buffer test_buffer;
    //ws_->read(test_buffer);
    //std::string test_msg = beast::buffers_to_string(test_buffer.data());
    //std::cout << "Remote candidate: " << test_msg << std::endl;
    //j = nlohmann::json::parse(msg);
    //rtc::Candidate candidate2(j["candidate"]["candidate"].get<std::string>(), j["candidate"]["sdpMid"].get<std::string>());
    //pc_->addRemoteCandidate(candidate2);
    //std::cout << "RemoteCandidate1 set successfully!" << std::endl;
} 
catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void RtpVideoStreamer::receiveLoop() 
{
    running_ = true;
    constexpr size_t BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];

    while (running_) {
        int len = recv(sock_, buffer, BUFFER_SIZE, 0);
        if (len < static_cast<int>(sizeof(rtc::RtpHeader)) || !track_->isOpen()) {
            continue;
        }

        auto rtp = reinterpret_cast<rtc::RtpHeader*>(buffer);
        rtp->setSsrc(ssrc_);

        track_->send(reinterpret_cast<const std::byte*>(buffer), len);
    }
}
