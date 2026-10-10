#include "v2/handlers/RelayHandler.hpp"

#include <cstring>
#include <vector>

namespace v2 {

RelayHandler::RelayHandler(IUserRepository& users, ITransport& transport) : users_(users), transport_(transport) {
    logger_->info("RelayHandler (v2) created");
}

void RelayHandler::relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) {
    const auto sender_id = users_.find_id_by_endpoint(sender);
    if (!sender_id.has_value()) {
        logger_->warn("Relay from unknown sender, dropping");
        return;
    }

    std::vector<uint8_t> outgoing(1 + data.size());
    outgoing[0] = type;
    std::memcpy(outgoing.data() + 1, data.data(), data.size());

    users_.for_each_except(*sender_id,
                           [this, &outgoing](const UserInfo& user) { transport_.send_to(user.endpoint, outgoing); });
}

}  // namespace v2
