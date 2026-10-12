#pragma once

#include "core/ConsoleLogger.hpp"
#include "net/ITransport.hpp"

#include <array>
#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>
#include <asio/strand.hpp>
#include <memory>

class UdpTransport final : public ITransport {
public:
    explicit UdpTransport(asio::io_context& io_ctx)
        : io_ctx_(io_ctx), strand_(asio::make_strand(io_ctx)), socket_(io_ctx) {
        logger_->info("UdpTransport created");
    }

    void start(udp::endpoint local) override;

    void send_to(const udp::endpoint& dest, std::span<const uint8_t> data) override;

    void set_receive_callback(std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet) override;

    void set_tick_callback(std::function<void()> callback, unsigned int interval_ms) override;

    void stop() override;

private:
    void async_receive();

    /// Reschedules the tick timer for the next interval.
    void schedule_tick();

    asio::io_context& io_ctx_;

    asio::strand<asio::io_context::executor_type> strand_;

    asio::ip::udp::socket socket_;

    std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet_;

    /// Tick timer for periodic callbacks (session sweep).
    std::unique_ptr<asio::steady_timer> tick_timer_;

    /// Callback invoked periodically by the tick timer.
    std::function<void()> on_tick_;

    /// Tick interval in milliseconds.
    unsigned int tick_interval_ms_{};

    static constexpr size_t recv_buf_size = 4096;
    using RecvBuffer = std::array<uint8_t, recv_buf_size>;

    static inline ILogger* logger_ = new ConsoleLogger();
};
