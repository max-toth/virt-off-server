#include "v2/game/SessionMonitor.hpp"
#include "v2/game/GameConstants.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <cstring>
#include <vector>

namespace v2::game {

SessionMonitor::SessionMonitor(IUserRepository& users,
                               IPropRepository& props,
                               IBroadcastHandler& broadcast)
    : users_(users), props_(props), broadcast_(broadcast) {}

void SessionMonitor::mark_active(const std::string& id) {
    users_.touch(id, std::chrono::system_clock::now());
}

void SessionMonitor::sweep(std::chrono::system_clock::time_point now) {
    const auto timeout = std::chrono::milliseconds(kSessionTimeoutMs);

    // Collect idle users (snapshot before remove_idle mutates the map).
    std::vector<std::string> idle_ids;
    users_.for_each([&](const UserInfo& user) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - user.last_seen);
        if (elapsed > timeout) {
            idle_ids.push_back(user.id);
        }
    });

    for (const auto& id : idle_ids) {
        // Release all props owned by this user.
        props_.release_all_of(id);

        // Broadcast PROP_RELEASE_EVENT for each released prop.
        props_.for_each([&](const PropRecord& rec) {
            if (rec.owner_id.empty()) {
                // Payload: [propId:4][ownerId:str][vel:12][angVel:12][flags:1]
                // ownerId is empty (len=0, no content) — FREE.
                const size_t payload_size = kU32Bytes + kStrLenBytes + kVec3Bytes + kVec3Bytes + kFlagsBytes;
                std::vector<uint8_t> payload(payload_size, 0);
                std::memcpy(payload.data(), &rec.prop_id, kU32Bytes);
                // owner_id len = 0 (FREE) — already zero from vector init.
                payload.back() = kPropFlagSettled;
                broadcast_.broadcast(protocol::MSG_PROP_RELEASE_EVENT, payload);
            }
        });

        // Remove user.
        users_.remove(id);
    }
}

}  // namespace v2::game
