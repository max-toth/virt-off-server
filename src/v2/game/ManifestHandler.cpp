#include "v2/game/ManifestHandler.hpp"
#include "v2/game/PropDescriptor.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <cstring>
#include <numeric>
#include <vector>

namespace v2::game {

ManifestHandler::ManifestHandler(IUserRepository& users,
                                 IPropRepository& props,
                                 IBroadcastHandler& broadcast)
    : users_(users), props_(props), broadcast_(broadcast) {}

void ManifestHandler::handle(const udp::endpoint& from, std::span<const uint8_t> payload) {
    const auto sender_id = users_.find_id_by_endpoint(from);
    if (!sender_id.has_value()) {
        return;
    }

    // Parse manifest: [id:str][count:2][{keyLen:1|key|halfExtents:12|mass:4}*]
    if (payload.size() < kStrLenBytes + kU16Bytes) {
        logger_->warn("[Manifest] from=" + *sender_id + " too short");
        return;
    }
    const uint8_t id_len = payload[0];
    if (payload.size() < kStrLenBytes + id_len + kU16Bytes) {
        return;
    }
    uint16_t count;
    std::memcpy(&count, payload.data() + kStrLenBytes + id_len, kU16Bytes);

    std::vector<PropDescriptor> descriptors;
    size_t offset = kStrLenBytes + id_len + kU16Bytes;
    for (uint16_t i = 0; i < count; ++i) {
        if (offset >= payload.size()) {
            logger_->warn("[Manifest] break at i=" + std::to_string(i) + " offset=" + std::to_string(offset) + " >= size=" + std::to_string(payload.size()));
            break;
        }
        const uint8_t key_len = payload[offset++];
        if (offset + key_len > payload.size()) {
            logger_->warn("[Manifest] break at i=" + std::to_string(i) + " key_len=" + std::to_string(key_len) + " offset=" + std::to_string(offset) + " > size=" + std::to_string(payload.size()));
            break;
        }
        PropDescriptor desc;
        desc.key.assign(reinterpret_cast<const char*>(payload.data() + offset), key_len);
        offset += key_len;

        if (offset + sizeof(desc.half_extents) > payload.size()) {
            logger_->warn("[Manifest] break half_extents at i=" + std::to_string(i));
            break;
        }
        std::memcpy(desc.half_extents, payload.data() + offset, sizeof(desc.half_extents));
        offset += sizeof(desc.half_extents);

        if (offset + sizeof(desc.mass) > payload.size()) {
            logger_->warn("[Manifest] break mass at i=" + std::to_string(i));
            break;
        }
        std::memcpy(&desc.mass, payload.data() + offset, sizeof(desc.mass));
        offset += sizeof(desc.mass);

        descriptors.push_back(std::move(desc));
    }

    logger_->info("[Manifest] from=" + *sender_id + " count=" + std::to_string(count) +
                  " parsed=" + std::to_string(descriptors.size()) + " offset_end=" + std::to_string(offset) +
                  " payload_size=" + std::to_string(payload.size()));

    props_.register_manifest(descriptors);

    // Build MANIFEST_ACK: [count:2][propId:4 * count]
    const size_t ack_size = kU16Bytes + count * kU32Bytes;
    std::vector<uint8_t> ack(ack_size);
    std::memcpy(ack.data(), &count, kU16Bytes);
    for (uint16_t i = 0; i < count; ++i) {
        const uint32_t prop_id = static_cast<uint32_t>(i + 1);
        std::memcpy(ack.data() + kU16Bytes + i * kU32Bytes, &prop_id, kU32Bytes);
    }

    broadcast_.send_to(*sender_id, protocol::MSG_MANIFEST_ACK, ack);
}

}  // namespace v2::game
