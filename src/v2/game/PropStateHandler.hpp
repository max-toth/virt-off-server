#pragma once

#include "v2/game/IPropStateHandler.hpp"

#include "v2/game/IBroadcastHandler.hpp"
#include "v2/game/IPropRepository.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "core/ConsoleLogger.hpp"

namespace v2::game {

/// Handles 0x04 PROP_STATE: validates ownership, broadcasts to all.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class PropStateHandler final : public IPropStateHandler {
public:
    PropStateHandler(IUserRepository& users,
                     IPropRepository& props,
                     IBroadcastHandler& broadcast);

    void handle(const udp::endpoint& from, std::span<const uint8_t> payload) override;

private:
    IUserRepository& users_;

    IPropRepository& props_;

    IBroadcastHandler& broadcast_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2::game
