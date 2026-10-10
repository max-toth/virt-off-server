#include "v2/app/Server.hpp"

#include "net/ProtocolConstants.hpp"

#include <span>
#include <string>
#include <utility>
#include <vector>

namespace v2 {

Server::Server(ServerDependencies deps, uint16_t port)
    : io_ctx_(std::move(deps.io_ctx))
    , id_generator_(std::move(deps.id_generator))
    , user_repository_(std::move(deps.user_repository))
    , transport_(std::move(deps.transport))
    , router_(std::move(deps.router))
    , connect_handler_(std::move(deps.connect_handler))
    , relay_handler_(std::move(deps.relay_handler))
    , port_(port) {
    logger_->info("Server (v2) object created, port=" + std::to_string(port_));
}

Server::~Server() {
    Stop();
}

void Server::Start() {
    logger_->info("Starting server (v2) on port " + std::to_string(port_));

    auto local = udp::endpoint(asio::ip::udp::v4(), port_);
    transport_->start(local);

    router_->register_handler(
        protocol::MSG_CONNECT,
        [this](const udp::endpoint& from, std::span<const uint8_t> /*payload*/) { connect_handler_->handle(from); });

    router_->register_handler(protocol::MSG_ASSIGN_ID,
                              [](const udp::endpoint& /*from*/, std::span<const uint8_t> /*payload*/) {});

    router_->register_handler(protocol::MSG_VOICE, [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
        relay_handler_->relay(protocol::MSG_VOICE, from, payload);
    });

    router_->register_handler(protocol::MSG_POSITION,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  relay_handler_->relay(protocol::MSG_POSITION, from, payload);
                              });

    transport_->set_receive_callback(
        [this](udp::endpoint from, std::vector<uint8_t> data) { router_->route(from, data); });

    logger_->info("Server (v2) is running");
}

void Server::Stop() noexcept {
    if (transport_) {
        transport_->stop();
    }

    if (io_ctx_ && !io_ctx_->stopped()) {
        io_ctx_->stop();
    }
}

}  // namespace v2
