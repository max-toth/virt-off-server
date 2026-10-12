#pragma once

#include "v2/game/PlayerTransform.hpp"

#include <optional>
#include <string>

namespace v2::game {

/// Tracks the latest transform of each player.
///
/// Used by IAttackGate for distance validation. Writing happens on
/// every POSITION packet; reading on every ATTACK.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IPlayerState {
    virtual ~IPlayerState() = default;

    /// Stores or replaces the transform for `id`.
    virtual void update(const std::string& id, const PlayerTransform& transform) = 0;

    /// Returns the latest known transform for `id`, or nullopt if unknown.
    virtual std::optional<PlayerTransform> get(const std::string& id) const = 0;

    /// Removes the transform for `id` (used on disconnect/timeout).
    virtual void erase(const std::string& id) = 0;
};

}  // namespace v2::game
