#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace v2::game {

/// Sends packets to all connected users (INCLUDING the author).
///
/// Unlike IRelayHandler which excludes the sender, broadcast includes
/// everyone — used for PROP_GRANT, PROP_RELEASE_EVENT, MANIFEST_ACK.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IBroadcastHandler {
    virtual ~IBroadcastHandler() = default;

    /// Sends `data` (without type byte) to ALL users, prepending `type`.
    virtual void broadcast(uint8_t type, std::span<const uint8_t> data) = 0;

    /// Sends `data` (without type byte) to a single user by `id`.
    virtual void send_to(const std::string& id, uint8_t type, std::span<const uint8_t> data) = 0;
};

}  // namespace v2::game
