#pragma once

#include "v2/game/IAttackHandler.hpp"

#include "v2/game/IAttackGate.hpp"
#include "v2/game/IBroadcastHandler.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "core/ConsoleLogger.hpp"

namespace v2::game {

/// Handles 0x03 ATTACK: validates via IAttackGate, relays on success.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class AttackHandler final : public IAttackHandler {
public:
    AttackHandler(IUserRepository& users,
                  IAttackGate& gate,
                  IBroadcastHandler& broadcast);

    void handle(const udp::endpoint& from, std::span<const uint8_t> payload) override;

private:
    IUserRepository& users_;

    IAttackGate& gate_;

    IBroadcastHandler& broadcast_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2::game
