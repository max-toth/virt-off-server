#pragma once

#include <cstdint>

namespace v2::game {

/// Maximum distance a client must be from a prop to pick it up.
inline constexpr float kPickupDistance = 3.0f;

/// Maximum distance an attacker must be from a target to land a hit.
inline constexpr float kAttackRange = 5.0f;

/// Minimum interval (ms) between two attacks from the same attacker to the same target.
inline constexpr unsigned int kAttackCooldownMs = 500;

/// How long a client can be silent before the server drops the session.
inline constexpr unsigned int kSessionTimeoutMs = 30'000;

/// How often the server sweeps idle sessions (tick interval).
inline constexpr unsigned int kSweepIntervalMs = 5'000;

}  // namespace v2::game
