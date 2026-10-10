#pragma once

#include <asio/ip/udp.hpp>

namespace v2 {

using asio::ip::udp;

/// Initial-connection handler (control byte 0xFF).
///
/// Assigns a string id via IIdGenerator, registers a full UserInfo in
/// IUserRepository, and replies with the new API response carrying the id.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp).
struct IConnectHandler {
    virtual ~IConnectHandler() = default;

    /// Processes a connection request from `client_endpoint`.
    virtual void handle(const udp::endpoint& client_endpoint) = 0;
};

}  // namespace v2
