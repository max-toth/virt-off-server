#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2 {

using asio::ip::udp;

/// Forwards relayed packets to all connected users except the sender.
///
/// Pure relay — no inspection or modification of packet contents.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp).
struct IRelayHandler {
    virtual ~IRelayHandler() = default;

    /// Broadcasts `data` (without type byte) to every user except `sender`,
    /// prepending `type` so receivers can distinguish message kinds.
    virtual void relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) = 0;
};

}  // namespace v2
