#pragma once

#include "clients/UserInfo.hpp"

#include <chrono>
#include <functional>
#include <optional>
#include <string>

namespace v2 {

using asio::ip::udp;

/// Registry of connected clients keeping full UserInfo records.
///
/// Two indexes: by string id and by udp::endpoint, so both "who is this
/// endpoint" and "where to send to this id" are O(1). Separate from the
/// legacy ::IClientRepository (endpoint-only, numeric ids).
///
/// Thread-safety: REQUIRED — implementation must guard both indexes.
struct IUserRepository {
    virtual ~IUserRepository() = default;

    /// Stores `user`. An entry with the same id or endpoint is replaced.
    virtual void add(UserInfo user) = 0;

    /// Removes the user with `id`. Unknown id is a no-op.
    virtual void remove(const std::string& id) = 0;

    /// Returns a copy of the user with `id`, or std::nullopt if not found.
    virtual std::optional<UserInfo> find_by_id(const std::string& id) const = 0;

    /// Returns the id bound to `endpoint`, or std::nullopt if not found.
    virtual std::optional<std::string> find_id_by_endpoint(const udp::endpoint& endpoint) const = 0;

    /// Updates `last_seen` of the user with `id` to `now`. Unknown id: no-op.
    virtual void touch(const std::string& id, std::chrono::system_clock::time_point now) = 0;

    /// Sets `username` of the user with `id`. Unknown id: no-op.
    virtual void set_username(const std::string& id, std::string username) = 0;

    /// Invokes `fn` for every user whose id != `exclude_id` (relay broadcast).
    virtual void for_each_except(const std::string& exclude_id,
                                 const std::function<void(const UserInfo&)>& fn) const = 0;
};

}  // namespace v2
