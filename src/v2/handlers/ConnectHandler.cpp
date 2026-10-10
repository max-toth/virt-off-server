#include "v2/handlers/ConnectHandler.hpp"

#include "net/ProtocolConstants.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace v2 {

ConnectHandler::ConnectHandler(IIdGenerator& id_gen, IUserRepository& users, ITransport& transport)
    : id_gen_(id_gen), users_(users), transport_(transport) {
    logger_->info("ConnectHandler (v2) created");
}

void ConnectHandler::handle(const udp::endpoint& client_endpoint) {
    const std::string id = id_gen_.next();
    const auto now = std::chrono::system_clock::now();

    UserInfo user{};
    user.id = id;
    user.endpoint = client_endpoint;
    user.connected_at = now;
    user.last_seen = now;
    user.session_state = SessionState::Connected;
    users_.add(std::move(user));

    logger_->info("Client " + id + " connected");

    std::vector<uint8_t> response;
    response.reserve(1 + id.size());
    response.push_back(protocol::MSG_ASSIGN_ID);
    response.insert(response.end(), id.begin(), id.end());

    transport_.send_to(client_endpoint, response);
}

}  // namespace v2
