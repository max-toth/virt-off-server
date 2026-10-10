#include "app/ServerBuilder.hpp"

#include "app/Server.hpp"
#include "clients/ClientRepository.hpp"
#include "core/MonotonicIdGenerator.hpp"
#include "handlers/ConnectHandler.hpp"
#include "handlers/RelayHandler.hpp"
#include "net/PacketRouter.hpp"
#include "net/UdpTransport.hpp"

#include <memory>

ServerBuilder::ServerBuilder(asio::io_context& io_ctx, uint16_t port) : io_ctx_(io_ctx), port_(port) {}

std::unique_ptr<IServer> ServerBuilder::build() {
    auto id_gen = std::make_unique<MonotonicIdGenerator>();
    auto clients = std::make_unique<ClientRepository>();
    auto transport = std::make_unique<UdpTransport>(io_ctx_);
    auto router = std::make_unique<PacketRouter>();
    auto connect = std::make_unique<ConnectHandler>(*id_gen, *clients, *transport);
    auto relay = std::make_unique<RelayHandler>(*clients, *transport);

    ServerDependencies deps{
        .io_ctx = nullptr,
        .id_generator = std::move(id_gen),
        .client_repository = std::move(clients),
        .transport = std::move(transport),
        .router = std::move(router),
        .connect_handler = std::move(connect),
        .relay_handler = std::move(relay),
    };

    return std::make_unique<Server>(std::move(deps), port_);
}
