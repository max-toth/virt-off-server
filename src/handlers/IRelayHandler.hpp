#pragma once

#include <cstdint>
#include <span>

#include <asio/ip/udp.hpp>

using asio::ip::udp;

/// Forwards voice data to all connected clients except the sender.
///
/// This is a pure relay — no inspection or modification of packet contents.
///
/// Thread-safety: NOT required — called from the io_context thread only.
struct IRelayHandler {
    virtual ~IRelayHandler() = default;

    /// Broadcasts `data` (without type byte) to every connected client excluding `sender`.
    /// Prepends `type` to each relayed packet so receivers can distinguish message kinds.
    virtual void relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) = 0;
};
