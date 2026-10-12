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

    // Start tick timer if configured.
    if (on_tick_ && tick_interval_ms_ > 0) {
        tick_timer_ = std::make_unique<asio::steady_timer>(io_ctx_);
        schedule_tick();
    }

    async_receive();
}

void UdpTransport::schedule_tick() {
    tick_timer_->expires_after(std::chrono::milliseconds(tick_interval_ms_));
    tick_timer_->async_wait(asio::bind_executor(strand_, [this](std::error_code ec) {
        if (ec) return;
        if (on_tick_) {
            on_tick_();
        }
        schedule_tick();
    }));
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

void UdpTransport::set_tick_callback(std::function<void()> callback, unsigned int interval_ms) {
    on_tick_ = std::move(callback);
    tick_interval_ms_ = interval_ms;
}

void UdpTransport::stop() {
    std::error_code ec;
    if (tick_timer_) {
        tick_timer_->cancel(ec);
    }
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
