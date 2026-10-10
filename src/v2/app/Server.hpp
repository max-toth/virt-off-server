#pragma once

#include "app/Server.hpp"
#include "core/ConsoleLogger.hpp"
#include "net/IPacketRouter.hpp"
#include "net/ITransport.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "v2/core/IIdGenerator.hpp"
#include "v2/handlers/IConnectHandler.hpp"
#include "v2/handlers/IRelayHandler.hpp"

#include <asio/io_context.hpp>
#include <cstdint>
#include <memory>

namespace v2 {

/// Dependencies of the V2 server, mirroring the legacy ServerDependencies.
///
/// io_ctx may be null: the caller owns the event loop and is responsible for
/// stopping it. When set, the server stops it on Stop().
struct ServerDependencies {
    std::unique_ptr<asio::io_context> io_ctx;

    std::unique_ptr<IIdGenerator> id_generator;

    std::unique_ptr<IUserRepository> user_repository;

    std::unique_ptr<ITransport> transport;

    std::unique_ptr<IPacketRouter> router;

    std::unique_ptr<IConnectHandler> connect_handler;

    std::unique_ptr<IRelayHandler> relay_handler;
};

/// V2 composition root: registers V2 handlers on the router and runs the
/// transport. Implements the version-agnostic ::IServer.
class Server final : public IServer {
public:
    Server(ServerDependencies deps, uint16_t port);

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;

    ~Server() override;

    void Start() override;

    void Stop() noexcept override;

private:
    std::unique_ptr<asio::io_context> io_ctx_;

    std::unique_ptr<IIdGenerator> id_generator_;

    std::unique_ptr<IUserRepository> user_repository_;

    std::unique_ptr<ITransport> transport_;

    std::unique_ptr<IPacketRouter> router_;

    std::unique_ptr<IConnectHandler> connect_handler_;

    std::unique_ptr<IRelayHandler> relay_handler_;

    uint16_t port_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2
