#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

using asio::ip::udp;

/// Low-level UDP socket wrapper.
///
/// All socket operations (receive, send) are serialized through an internal
/// asio::strand so the implementation is safe to use from multiple threads
/// calling io_context::run(). Callbacks are always invoked on the strand.
///
/// Thread-safety: required — all public methods may be called from any thread.
struct ITransport {
    virtual ~ITransport() = default;

    /// Binds the UDP socket to `local` address.
    virtual void start(udp::endpoint local) = 0;

    /// Sends raw bytes to `dest`. Non-blocking — queues an async send.
    virtual void send_to(const udp::endpoint& dest, std::span<const uint8_t> data) = 0;

    /// Sets a callback invoked on each received packet.
    /// `on_packet(endpoint, payload)`
    virtual void set_receive_callback(std::function<void(udp::endpoint, std::vector<uint8_t>)> on_packet) = 0;

    /// Closes the socket and cancels any outstanding async operations.
    virtual void stop() = 0;
};
