#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

using asio::ip::udp;

/// Handles 0x04 PROP_STATE from a client.
/// Validates ownership (only owner may send state),
/// broadcasts to all (including author).
struct IPropStateHandler {
    virtual ~IPropStateHandler() = default;

    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
