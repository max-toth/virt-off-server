#pragma once

#include <asio/ip/udp.hpp>

using asio::ip::udp;

/// Handles initial client connection (control byte 0xFF).
///
/// On connect: assigns a new client ID via IIdGenerator, stores the client
/// endpoint in IClientRepository, and sends the ID back as a 0xFE packet.
///
/// Thread-safety: NOT required — called from the io_context thread only.
struct IConnectHandler {
    virtual ~IConnectHandler() = default;

    /// Processes a connection request from `client_endpoint`.
    virtual void handle(const udp::endpoint& client_endpoint) = 0;
};
