#pragma once

#include "v2/game/ISessionMonitor.hpp"

#include "v2/game/IBroadcastHandler.hpp"
#include "v2/game/IPropRepository.hpp"
#include "v2/clients/IUserRepository.hpp"

#include <chrono>
#include <string>

namespace v2::game {

/// Session liveness tracker.
///
/// mark_active is called on every incoming packet. sweep evicts idle sessions
/// and broadcasts PROP_RELEASE_EVENT for each dropped owner's props.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class SessionMonitor final : public ISessionMonitor {
public:
    SessionMonitor(IUserRepository& users,
                   IPropRepository& props,
                   IBroadcastHandler& broadcast);

    void mark_active(const std::string& id) override;

    void sweep(std::chrono::system_clock::time_point now) override;

private:
    IUserRepository& users_;

    IPropRepository& props_;

    IBroadcastHandler& broadcast_;
};

}  // namespace v2::game
