#include "v2/game/BroadcastHandler.hpp"

#include <cstring>

namespace v2::game {

BroadcastHandler::BroadcastHandler(IUserRepository& users, ITransport& transport)
    : users_(users), transport_(transport) {}

std::vector<uint8_t> BroadcastHandler::assemble_datagram(uint8_t type, std::span<const uint8_t> data) {
    std::vector<uint8_t> out(1 + data.size());
    out[0] = type;
    std::memcpy(out.data() + 1, data.data(), data.size());
    return out;
}

void BroadcastHandler::broadcast(uint8_t type, std::span<const uint8_t> data) {
    const auto datagram = assemble_datagram(type, data);
    users_.for_each([this, &datagram](const UserInfo& user) {
        transport_.send_to(user.endpoint, datagram);
    });
}

void BroadcastHandler::send_to(const std::string& id, uint8_t type, std::span<const uint8_t> data) {
    const auto user = users_.find_by_id(id);
    if (!user.has_value()) {
        return;
    }
    const auto datagram = assemble_datagram(type, data);
    transport_.send_to(user->endpoint, datagram);
}

}  // namespace v2::game
