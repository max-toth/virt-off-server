#include "v2/app/ServerBuilder.hpp"

#include "net/PacketRouter.hpp"
#include "net/UdpTransport.hpp"
#include "v2/app/Server.hpp"
#include "v2/clients/UserRepository.hpp"
#include "v2/core/TimestampIdGenerator.hpp"
#include "v2/handlers/ConnectHandler.hpp"
#include "v2/handlers/RelayHandler.hpp"

#include <memory>

namespace v2 {

ServerBuilder::ServerBuilder(asio::io_context& io_ctx, uint16_t port) : io_ctx_(io_ctx), port_(port) {}

std::unique_ptr<IServer> ServerBuilder::build() {
    auto id_gen = std::make_unique<TimestampIdGenerator>();
    auto users = std::make_unique<UserRepository>();
    auto transport = std::make_unique<UdpTransport>(io_ctx_);
    auto router = std::make_unique<PacketRouter>();
    auto connect = std::make_unique<ConnectHandler>(*id_gen, *users, *transport);
    auto relay = std::make_unique<RelayHandler>(*users, *transport);

    ServerDependencies deps{
        .io_ctx = nullptr,
        .id_generator = std::move(id_gen),
        .user_repository = std::move(users),
        .transport = std::move(transport),
        .router = std::move(router),
        .connect_handler = std::move(connect),
        .relay_handler = std::move(relay),
    };

    return std::make_unique<Server>(std::move(deps), port_);
}

}  // namespace v2
