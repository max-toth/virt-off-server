#include "net/UdpTransport.hpp"

#include <asio/bind_executor.hpp>
#include <system_error>

void UdpTransport::start(udp::endpoint local) {
    std::error_code ec;
    socket_.open(local.protocol(), ec);
    if (ec) {
        logger_->error("Failed to open UDP socket: " + ec.message());
        throw std::system_error(ec, "Failed to open UDP socket");
    }

    socket_.bind(local, ec);
    if (ec) {
        logger_->error("Failed to bind UDP socket: " + ec.message());
        throw std::system_error(ec, "Failed to bind UDP socket");
    }

    async_receive();
}

void UdpTransport::send_to(const udp::endpoint& dest, std::span<const uint8_t> data) {
    auto buf = std::make_shared<std::vector<uint8_t>>(data.begin(), data.end());

    asio::post(strand_, [this, dest, buf]() {
        socket_.async_send_to(asio::buffer(*buf), dest,
                              asio::bind_executor(strand_, [buf](std::error_code ec, std::size_t /*sent*/) {
                                  if (ec) {
                                      // send error silently ignored — UDP packet loss is expected
                                  }
                              }));
    });
}

void UdpTransport::set_receive_callback(std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet) {
    on_packet_ = std::move(on_packet);
}

void UdpTransport::stop() {
    std::error_code ec;
    socket_.close(ec);
}

void UdpTransport::async_receive() {
    auto buffer = std::make_shared<RecvBuffer>();
    auto remote = std::make_shared<udp::endpoint>();

    socket_.async_receive_from(
        asio::buffer(*buffer), *remote,
        asio::bind_executor(strand_, [this, buffer, remote](std::error_code ec, std::size_t bytes) {
            if (ec) return;

            if (on_packet_ && bytes > 0) {
                std::vector<uint8_t> payload(buffer->begin(), buffer->begin() + bytes);
                on_packet_(*remote, std::move(payload));
            }

            async_receive();
        }));
}
