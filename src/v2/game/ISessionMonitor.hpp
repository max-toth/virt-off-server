#pragma once

#include <chrono>
#include <string>

namespace v2::game {

/// Tracks client liveness and performs session timeout / cleanup.
///
/// mark_active is called on every incoming packet (receive wrapper).
/// sweep is called periodically (tick callback) to evict idle sessions.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct ISessionMonitor {
    virtual ~ISessionMonitor() = default;

    /// Updates the last-seen timestamp for `id` to now.
    virtual void mark_active(const std::string& id) = 0;

    /// Finds and drops sessions idle longer than kSessionTimeout.
    /// Calls IUserRepository::remove_idle + IPropRepository::release_all_of
    /// + IBroadcastHandler::broadcast for each dropped session.
    virtual void sweep(std::chrono::system_clock::time_point now) = 0;
};

}  // namespace v2::game
