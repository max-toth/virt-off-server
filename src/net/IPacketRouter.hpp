#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <vector>

#include <asio/ip/udp.hpp>

using asio::ip::udp;

/// Dispatches incoming packets to registered handlers by first byte.
///
/// Handlers are invoked synchronously inside route(). The caller is
/// responsible for ensuring the handler executes on an appropriate thread.
///
/// Thread-safety: NOT required — route() must be called from a single thread.
struct IPacketRouter {
    virtual ~IPacketRouter() = default;

    /// Registers a handler for packets whose first byte equals `type`.
    /// handler(endpoint, full_payload_without_first_byte)
    /// Only one handler per type is allowed. Calling twice for the same type
    /// replaces the previous handler.
    virtual void register_handler(uint8_t type,
                                  std::function<void(udp::endpoint, std::span<const uint8_t>)> handler) = 0;

    /// Dispatches `data` to the handler registered for `data[0]`.
    /// The first byte is stripped before passing to the handler.
    /// If no handler is registered for the type, the packet is silently dropped.
    virtual void route(const udp::endpoint& from, std::span<const uint8_t> data) = 0;
};
