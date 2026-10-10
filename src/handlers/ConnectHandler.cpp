#include "handlers/ConnectHandler.hpp"

#include <cstring>
#include <vector>

void ConnectHandler::handle(const udp::endpoint& client_endpoint)
{
    auto id = id_gen_.next();
    clients_.add(client_endpoint);

    logger_->info("Client " + std::to_string(id) + " connected");

    std::vector<uint8_t> response(9, 0);
    response[0] = 0xFE;
    std::memcpy(response.data() + 1, &id, sizeof(id));

    transport_.send_to(client_endpoint, response);
}
