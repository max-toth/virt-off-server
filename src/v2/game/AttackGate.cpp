#include "v2/game/AttackGate.hpp"
#include "v2/game/GameConstants.hpp"

#include <cmath>
#include <sstream>

namespace v2::game {

AttackGate::AttackGate(IPlayerState& player_state)
    : player_state_(player_state) {}

bool AttackGate::allow_attack(const std::string& attacker_id,
                               const std::string& target_id,
                               std::chrono::system_clock::time_point now) {
    // Build cooldown key.
    std::ostringstream key;
    key << attacker_id << ":" << target_id;
    const std::string cd_key = key.str();

    // Check cooldown.
    const auto it = cooldowns_.find(cd_key);
    if (it != cooldowns_.end()) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
        if (elapsed.count() < static_cast<long long>(kAttackCooldownMs)) {
            return false;
        }
    }

    // Check distance.
    const auto attacker_pos = player_state_.get(attacker_id);
    const auto target_pos = player_state_.get(target_id);
    if (!attacker_pos || !target_pos) {
        return false;
    }

    const float dx = attacker_pos->position[0] - target_pos->position[0];
    const float dy = attacker_pos->position[1] - target_pos->position[1];
    const float dz = attacker_pos->position[2] - target_pos->position[2];
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist > kAttackRange) {
        return false;
    }

    // Update cooldown.
    cooldowns_[cd_key] = now;
    return true;
}

}  // namespace v2::game
