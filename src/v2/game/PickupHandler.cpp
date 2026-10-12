#include "v2/game/PickupHandler.hpp"
#include "v2/game/GameConstants.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <cmath>
#include <cstring>
#include <vector>

namespace v2::game {

PickupHandler::PickupHandler(IUserRepository& users,
                             IPropRepository& props,
                             IPlayerState& player_state,
                             IBroadcastHandler& broadcast)
    : users_(users), props_(props), player_state_(player_state), broadcast_(broadcast) {}

void PickupHandler::handle(const udp::endpoint& from, std::span<const uint8_t> payload) {
    const auto sender_id = users_.find_id_by_endpoint(from);
    if (!sender_id.has_value()) {
        return;
    }

    // Payload: [id:str][propId:4]
    if (payload.size() < kStrLenBytes + kU32Bytes) {
        return;
    }
    const uint8_t id_len = payload[0];
    if (payload.size() < kStrLenBytes + id_len + kU32Bytes) {
        return;
    }
    uint32_t prop_id;
    std::memcpy(&prop_id, payload.data() + kStrLenBytes + id_len, kU32Bytes);

    const auto prop = props_.get(prop_id);
    if (!prop.has_value()) {
        logger_->warn("[Pickup] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " not found");
        return;
    }

    // Distance check.
    const auto player_pos = player_state_.get(*sender_id);
    if (!player_pos.has_value()) {
        logger_->warn("[Pickup] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " no position");
        return;
    }
    const float dx = player_pos->position[0];
    const float dy = player_pos->position[1];
    const float dz = player_pos->position[2];
    const float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dist > kPickupDistance) {
        logger_->info("[Pickup] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " too far dist=" + std::to_string(dist));
        return;
    }

    if (!props_.try_grant(prop_id, *sender_id)) {
        logger_->info("[Pickup] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " already held");
        return;
    }

    logger_->info("[Pickup] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " GRANT");

    // Broadcast PROP_GRANT: [propId:4][holderId:str][flags:1]
    const auto& holder_id = *sender_id;
    const size_t grant_size = kU32Bytes + kStrLenBytes + holder_id.size() + kFlagsBytes;
    std::vector<uint8_t> grant(grant_size, 0);
    std::memcpy(grant.data(), &prop_id, kU32Bytes);
    grant[kU32Bytes] = static_cast<uint8_t>(holder_id.size());
    std::memcpy(grant.data() + kU32Bytes + kStrLenBytes, holder_id.data(), holder_id.size());

    broadcast_.broadcast(protocol::MSG_PROP_GRANT, grant);
}

}  // namespace v2::game
