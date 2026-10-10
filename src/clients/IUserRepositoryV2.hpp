#pragma once

#include "clients/UserInfo.hpp"

#include <chrono>
#include <functional>
#include <optional>
#include <string>

/// V2 registry of connected clients, keeping full UserInfo records.
///
/// Maintains two indexes: by string id and by udp::endpoint, so both
/// "who is this endpoint" (relay source lookup) and "where do I send to
/// this id" (direct send) are O(1). Kept separate from the V1
/// IClientRepository for backward compatibility.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp). Implementation must guard
/// both indexes with a mutex.
struct IUserRepositoryV2 {
    virtual ~IUserRepositoryV2() = default;

    /// Stores `user`. If an entry with the same id or endpoint already
    /// exists, that entry is replaced.
    virtual void add(UserInfo user) = 0;

    /// Removes the user with `id`. Safe to call with an unknown id (no-op).
    virtual void remove(const std::string& id) = 0;

    /// Returns a copy of the user with `id`, or std::nullopt if not found.
    virtual std::optional<UserInfo> find_by_id(const std::string& id) const = 0;

    /// Returns the id bound to `endpoint`, or std::nullopt if not found.
    /// Used to identify the sender of an incoming packet.
    virtual std::optional<std::string> find_id_by_endpoint(const udp::endpoint& endpoint) const = 0;

    /// Updates `last_seen` of the user with `id` to `now`.
    /// Safe to call with an unknown id (no-op).
    virtual void touch(const std::string& id, std::chrono::system_clock::time_point now) = 0;

    /// Sets `username` of the user with `id`.
    /// Safe to call with an unknown id (no-op).
    virtual void set_username(const std::string& id, std::string username) = 0;

    /// Invokes `fn` for every user whose id is NOT equal to `exclude_id`.
    /// Used for relay broadcasting to all clients except the sender.
    virtual void for_each_except(const std::string& exclude_id,
                                 const std::function<void(const UserInfo&)>& fn) const = 0;
};
