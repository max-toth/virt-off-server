#pragma once

#include <asio/ip/udp.hpp>

using asio::ip::udp;

/// V2 initial-connection handler (control byte 0xFF).
///
/// On connect: assigns a string id via IIdGeneratorV2, registers a full
/// UserInfo (id, endpoint, connected_at, last_seen) in IUserRepositoryV2,
/// and replies with the new server API response carrying the string id.
/// Kept separate from the V1 IConnectHandler for backward compatibility.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp).
struct IConnectHandlerV2 {
    virtual ~IConnectHandlerV2() = default;

    /// Processes a connection request from `client_endpoint`.
    virtual void handle(const udp::endpoint& client_endpoint) = 0;
};
