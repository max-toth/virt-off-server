#pragma once

#include <chrono>
#include <string>

namespace v2::game {

/// Validates whether an attack between two players is allowed.
///
/// Currently holds two checks:
///   1. Cooldown — attacker cannot hit the same target too frequently.
///   2. Distance — attacker must be within range of target.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IAttackGate {
    virtual ~IAttackGate() = default;

    /// Returns true if `attacker_id` may hit `target_id` at `now`.
    virtual bool allow_attack(const std::string& attacker_id,
                              const std::string& target_id,
                              std::chrono::system_clock::time_point now) = 0;
};

}  // namespace v2::game
