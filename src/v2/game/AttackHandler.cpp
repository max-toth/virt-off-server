#include "v2/game/AttackHandler.hpp"
#include "v2/game/WireSizes.hpp"
#include "net/ProtocolConstants.hpp"

#include <chrono>
#include <cstring>
#include <vector>

namespace v2::game {

AttackHandler::AttackHandler(IUserRepository& users,
                             IAttackGate& gate,
                             IBroadcastHandler& broadcast)
    : users_(users), gate_(gate), broadcast_(broadcast) {}

void AttackHandler::handle(const udp::endpoint& from, std::span<const uint8_t> payload) {
    const auto sender_id = users_.find_id_by_endpoint(from);
    if (!sender_id.has_value()) {
        return;
    }

    // Payload: [id:str][kind:1][targetId:str][point:12][impulse:12][hitObject:1]
    if (payload.size() < kStrLenBytes + kKindBytes) {
        return;
    }

    const uint8_t sender_id_len = payload[0];
    if (payload.size() < kStrLenBytes + sender_id_len + kKindBytes + kStrLenBytes) {
        return;
    }

    // Parse target id (length-prefixed string after kind byte).
    const uint8_t target_len = payload[kStrLenBytes + sender_id_len + kKindBytes];
    if (payload.size() < kStrLenBytes + sender_id_len + kKindBytes + kStrLenBytes + target_len) {
        return;
    }
    const size_t target_off = kStrLenBytes + sender_id_len + kKindBytes + kStrLenBytes;
    const std::string target_id(reinterpret_cast<const char*>(payload.data() + target_off), target_len);

    const auto now = std::chrono::system_clock::now();
    if (!gate_.allow_attack(*sender_id, target_id, now)) {
        logger_->info("[Attack] from=" + *sender_id + " target=" + target_id + " blocked (cooldown/distance)");
        return;
    }

    logger_->info("[Attack] from=" + *sender_id + " target=" + target_id);

    // Relay the original payload to all (including author for feedback).
    broadcast_.broadcast(protocol::MSG_ATTACK, payload);
}

}  // namespace v2::game
