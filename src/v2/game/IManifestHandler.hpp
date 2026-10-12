#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

using asio::ip::udp;

/// Handles 0x06 MANIFEST from a client.
/// Parses prop descriptors, calls IPropRepository::register_manifest,
/// replies with MANIFEST_ACK containing assigned prop IDs.
struct IManifestHandler {
    virtual ~IManifestHandler() = default;

    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
