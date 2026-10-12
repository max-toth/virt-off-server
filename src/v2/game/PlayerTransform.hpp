#pragma once

#include <cstdint>

namespace v2::game {

/// Snapshot of a player's spatial state for attack validation.
struct PlayerTransform {
    /// World-space position (x, y, z).
    float position[3]{};

    /// Quaternion (x, y, z, w).
    float rotation[4]{};

    /// Linear velocity (x, y, z).
    float velocity[3]{};
};

}  // namespace v2::game
