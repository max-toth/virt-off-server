#pragma once

#include "net/ITransport.hpp"
#include "core/ConsoleLogger.hpp"

#include <asio/io_context.hpp>
#include <asio/strand.hpp>

#include <array>
#include <memory>

class UdpTransport final : public ITransport {
public:
    explicit UdpTransport(asio::io_context& io_ctx)
        : io_ctx_(io_ctx), strand_(asio::make_strand(io_ctx)), socket_(io_ctx)
    {
        logger_->info("UdpTransport created");
    }

    void start(udp::endpoint local) override;

    void send_to(const udp::endpoint& dest, std::span<const uint8_t> data) override;

    void set_receive_callback(
        std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet) override;

    void stop() override;

private:
    void async_receive();

    asio::io_context& io_ctx_;

    asio::strand<asio::io_context::executor_type> strand_;

    asio::ip::udp::socket socket_;

    std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet_;

    static constexpr size_t recv_buf_size = 4096;
    using RecvBuffer = std::array<uint8_t, recv_buf_size>;

    static inline ILogger* logger_ = new ConsoleLogger();
};
