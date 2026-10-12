#include "v2/game/PropStateHandler.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <cstring>
#include <vector>

namespace v2::game {

PropStateHandler::PropStateHandler(IUserRepository& users,
                                   IPropRepository& props,
                                   IBroadcastHandler& broadcast)
    : users_(users), props_(props), broadcast_(broadcast) {}

void PropStateHandler::handle(const udp::endpoint& from, std::span<const uint8_t> payload) {
    const auto sender_id = users_.find_id_by_endpoint(from);
    if (!sender_id.has_value()) {
        return;
    }

    // Payload: [id:str][propId:4][pos:12][rot:16][vel:12][flags:1]
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
        return;
    }

    // Only owner may send state.
    if (prop->owner_id != *sender_id) {
        logger_->warn("[PropState] from=" + *sender_id + " prop=" + std::to_string(prop_id) + " not owner");
        return;
    }

    // Broadcast to all including author.
    broadcast_.broadcast(protocol::MSG_PROP_STATE, payload);
}

}  // namespace v2::game
