#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

using asio::ip::udp;

/// Handles 0x08 PROP_RELEASE from a client.
/// Validates ownership, calls IPropRepository::release,
/// broadcasts PROP_RELEASE_EVENT.
struct IReleaseHandler {
    virtual ~IReleaseHandler() = default;

    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
