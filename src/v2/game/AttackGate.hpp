#pragma once

#include "v2/game/IAttackGate.hpp"
#include "v2/game/IPlayerState.hpp"

#include <string>
#include <unordered_map>

namespace v2::game {

/// Attack gate that enforces cooldown and distance.
///
/// Cooldown is tracked per (attacker, target) pair. Distance is computed from
/// the transforms stored in IPlayerState.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class AttackGate final : public IAttackGate {
public:
    explicit AttackGate(IPlayerState& player_state);

    bool allow_attack(const std::string& attacker_id,
                      const std::string& target_id,
                      std::chrono::system_clock::time_point now) override;

private:
    /// Reference to the player transform registry.
    IPlayerState& player_state_;

    /// Cooldown map: "attacker:target" → last allowed time.
    std::unordered_map<std::string, std::chrono::system_clock::time_point> cooldowns_;
};

}  // namespace v2::game
