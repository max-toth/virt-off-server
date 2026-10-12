#include "v2/game/ReleaseHandler.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <cstring>
#include <vector>

namespace v2::game {

ReleaseHandler::ReleaseHandler(IUserRepository& users,
                               IPropRepository& props,
                               IBroadcastHandler& broadcast)
    : users_(users), props_(props), broadcast_(broadcast) {}

void ReleaseHandler::handle(const udp::endpoint& from, std::span<const uint8_t> payload) {
    const auto sender_id = users_.find_id_by_endpoint(from);
    if (!sender_id.has_value()) {
        return;
    }

    // Payload: [id:str][propId:4][vel:12][angVel:12][flags:1]
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
        logger_->warn("[Release] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " not found");
        return;
    }

    // Only owner may release.
    if (prop->owner_id != *sender_id) {
        logger_->warn("[Release] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " not owner (owner=" + prop->owner_id + ")");
        return;
    }

    props_.release(prop_id);
    logger_->info("[Release] from=" + *sender_id + " prop=" + std::to_string(prop_id));

    // Broadcast PROP_RELEASE_EVENT: [propId:4][ownerId:str][vel:12][angVel:12][flags:1]
    // Original payload is [id:str][propId:4][vel:12][angVel:12][flags:1].
    // We drop the sender id and replace it with ownerId (= sender_id).
    const size_t vel_off = kStrLenBytes + id_len + kU32Bytes;
    const size_t angvel_off = vel_off + kVec3Bytes;
    const size_t flags_off = angvel_off + kVec3Bytes;
    const size_t event_size = kU32Bytes + kStrLenBytes + sender_id->size() + kVec3Bytes + kVec3Bytes + kFlagsBytes;
    std::vector<uint8_t> event(event_size);
    // Copy propId.
    std::memcpy(event.data(), &prop_id, kU32Bytes);
    // Write ownerId (length-prefixed).
    event[kU32Bytes] = static_cast<uint8_t>(sender_id->size());
    std::memcpy(event.data() + kU32Bytes + kStrLenBytes, sender_id->data(), sender_id->size());
    // Copy vel, angVel, flags from original payload.
    std::memcpy(event.data() + kU32Bytes + kStrLenBytes + sender_id->size(), payload.data() + vel_off, kVec3Bytes + kVec3Bytes + kFlagsBytes);
    broadcast_.broadcast(protocol::MSG_PROP_RELEASE_EVENT, event);
}

}  // namespace v2::game
