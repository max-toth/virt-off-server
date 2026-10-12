#include "v2/game/PlayerState.hpp"

namespace v2::game {

void PlayerState::update(const std::string& id, const PlayerTransform& transform) {
    entries_[id] = transform;
}

std::optional<PlayerTransform> PlayerState::get(const std::string& id) const {
    const auto it = entries_.find(id);
    if (it == entries_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void PlayerState::erase(const std::string& id) {
    entries_.erase(id);
}

}  // namespace v2::game
