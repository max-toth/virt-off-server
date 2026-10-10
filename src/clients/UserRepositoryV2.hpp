#pragma once

#include "clients/IUserRepositoryV2.hpp"

#include <mutex>
#include <unordered_map>

/// Thread-safe V2 client registry keeping full UserInfo records.
///
/// Two indexes are kept in sync under a single mutex:
///   by_id_       — id -> record, for direct send / lookup by id
///   by_endpoint_ — endpoint -> id, for identifying the sender of a packet
class UserRepositoryV2 final : public IUserRepositoryV2 {
public:
    void add(UserInfo user) override;

    void remove(const std::string& id) override;

    std::optional<UserInfo> find_by_id(const std::string& id) const override;

    std::optional<std::string> find_id_by_endpoint(const udp::endpoint& endpoint) const override;

    void touch(const std::string& id, std::chrono::system_clock::time_point now) override;

    void set_username(const std::string& id, std::string username) override;

    void for_each_except(const std::string& exclude_id, const std::function<void(const UserInfo&)>& fn) const override;

private:
    /// Guards both indexes.
    mutable std::mutex mutex_;

    /// Primary index: id -> full record.
    std::unordered_map<std::string, UserInfo> by_id_;

    /// Secondary index: endpoint -> id.
    std::unordered_map<udp::endpoint, std::string> by_endpoint_;
};
