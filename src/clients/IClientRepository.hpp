#pragma once

#include <cstdint>
#include <functional>
#include <optional>

#include <asio/ip/udp.hpp>

using asio::ip::udp;

/// Registry of all currently connected clients.
///
/// Thread-safety: NOT required — called from the io_context thread only.
/// Future: must become thread-safe (mutex or concurrent map) when multiple
/// threads run io_context.
struct IClientRepository {
    virtual ~IClientRepository() = default;

    /// Registers a client endpoint and returns the assigned ID.
    virtual uint64_t add(udp::endpoint ep) = 0;

    /// Removes a client by ID. Safe to call with an unknown ID.
    virtual void remove(uint64_t id) = 0;

    /// Finds a client ID by endpoint. Returns std::nullopt if not found.
    virtual std::optional<uint64_t> find_by_endpoint(const udp::endpoint& ep) const = 0;

    /// Invokes `fn(id, endpoint)` for every connected client whose ID is
    /// NOT equal to `exclude_id`. Useful for relay broadcasting.
    virtual void for_each_except(uint64_t exclude_id,
                                 std::function<void(uint64_t, const udp::endpoint&)> fn) const = 0;
};
