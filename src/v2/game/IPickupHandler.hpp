#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

using asio::ip::udp;

/// Handles 0x07 PICKUP_REQ from a client.
/// Validates distance, calls IPropRepository::try_grant,
/// broadcasts PROP_GRANT on success.
struct IPickupHandler {
    virtual ~IPickupHandler() = default;

    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
