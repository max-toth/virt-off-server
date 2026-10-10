#pragma once

#include <asio/ip/udp.hpp>
#include <chrono>
#include <string>

using asio::ip::udp;

/// Session lifecycle state of a connected client (V2).
enum class SessionState {
    /// Client has an active session and participates in relay.
    Connected,

    /// Client session was terminated (removed or timed out).
    Disconnected,
};

/// V2 in-memory record describing a single connected client.
///
/// Plain data holder — no behaviour. Owned by the V2 user repository.
struct UserInfo {
    /// Unique client identifier produced by IIdGeneratorV2, format "player-<...>".
    std::string id;

    /// Display name reported by the client. Empty until the client sends it.
    std::string username;

    /// Client UDP address (IP + port) used for routing and relay.
    udp::endpoint endpoint;

    /// Time when the client connection was registered.
    std::chrono::system_clock::time_point connected_at;

    /// Time of the last packet received from the client, used for idle timeout.
    std::chrono::system_clock::time_point last_seen;

    /// True while the client is transmitting voice; derived from voice packets.
    bool is_speaking = false;

    /// Current lifecycle state of the client session.
    SessionState session_state = SessionState::Connected;
};
