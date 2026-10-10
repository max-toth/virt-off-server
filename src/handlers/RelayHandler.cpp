#include "handlers/RelayHandler.hpp"

RelayHandler::RelayHandler(IClientRepository& clients, ITransport& transport)
    : clients_(clients), transport_(transport) {
    logger_->info("RelayHandler initialized");
}

void RelayHandler::relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) {
    auto sender_id = clients_.find_by_endpoint(sender);
    if (!sender_id.has_value()) {
        logger_->warn("Relay from unknown sender, dropping");
        return;
    }

    std::vector<uint8_t> outgoing(1 + data.size());
    outgoing[0] = type;
    std::memcpy(outgoing.data() + 1, data.data(), data.size());

    uint64_t sender_val = *sender_id;
    clients_.for_each_except(
        sender_val, [this, &outgoing](uint64_t /*id*/, const udp::endpoint& ep) { transport_.send_to(ep, outgoing); });
}
