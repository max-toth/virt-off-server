#pragma once

#include "v2/game/IPlayerState.hpp"

#include <string>
#include <unordered_map>

namespace v2::game {

/// In-memory player transform registry.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class PlayerState final : public IPlayerState {
public:
    void update(const std::string& id, const PlayerTransform& transform) override;

    std::optional<PlayerTransform> get(const std::string& id) const override;

    void erase(const std::string& id) override;

private:
    /// id → latest transform.
    std::unordered_map<std::string, PlayerTransform> entries_;
};

}  // namespace v2::game
