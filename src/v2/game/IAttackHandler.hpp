#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

using asio::ip::udp;

/// Handles 0x03 ATTACK from a client.
/// Validates via IAttackGate, relays to target on success.
struct IAttackHandler {
    virtual ~IAttackHandler() = default;

    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
