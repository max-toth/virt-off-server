#include "v2/clients/UserRepository.hpp"

#include <utility>
#include <vector>

namespace v2 {

void UserRepository::add(UserInfo user) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Drop any previous record bound to the same endpoint.
    const auto ep_it = by_endpoint_.find(user.endpoint);
    if (ep_it != by_endpoint_.end()) {
        by_id_.erase(ep_it->second);
        by_endpoint_.erase(ep_it);
    }

    // Drop any previous record carrying the same id.
    const auto id_it = by_id_.find(user.id);
    if (id_it != by_id_.end()) {
        by_endpoint_.erase(id_it->second.endpoint);
        by_id_.erase(id_it);
    }

    by_endpoint_[user.endpoint] = user.id;
    by_id_[user.id] = std::move(user);
}

void UserRepository::remove(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = by_id_.find(id);
    if (it == by_id_.end()) {
        return;
    }

    by_endpoint_.erase(it->second.endpoint);
    by_id_.erase(it);
}

std::optional<UserInfo> UserRepository::find_by_id(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = by_id_.find(id);
    if (it == by_id_.end()) {
        return std::nullopt;
    }

    return it->second;
}

std::optional<std::string> UserRepository::find_id_by_endpoint(const udp::endpoint& endpoint) const {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = by_endpoint_.find(endpoint);
    if (it == by_endpoint_.end()) {
        return std::nullopt;
    }

    return it->second;
}

void UserRepository::touch(const std::string& id, std::chrono::system_clock::time_point now) {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = by_id_.find(id);
    if (it == by_id_.end()) {
        return;
    }

    it->second.last_seen = now;
}

void UserRepository::set_username(const std::string& id, std::string username) {
    std::lock_guard<std::mutex> lock(mutex_);

    const auto it = by_id_.find(id);
    if (it == by_id_.end()) {
        return;
    }

    it->second.username = std::move(username);
}

void UserRepository::for_each_except(const std::string& exclude_id,
                                     const std::function<void(const UserInfo&)>& fn) const {
    // Snapshot targets under the lock, then invoke callbacks unlocked so a
    // callback that re-enters the repository cannot deadlock.
    std::vector<UserInfo> targets;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        targets.reserve(by_id_.size());
        for (const auto& [id, user] : by_id_) {
            if (id != exclude_id) {
                targets.push_back(user);
            }
        }
    }

    for (const auto& user : targets) {
        fn(user);
    }
}

}  // namespace v2
