#pragma once

#include <memory>

#include <asio/io_context.hpp>

#include "core/IIdGenerator.hpp"
#include "core/ConsoleLogger.hpp"
#include "clients/IClientRepository.hpp"
#include "net/ITransport.hpp"
#include "net/IPacketRouter.hpp"
#include "handlers/IConnectHandler.hpp"
#include "handlers/IRelayHandler.hpp"

struct IServer {
    virtual ~IServer() = default;

    /// Sets up handlers and starts listening on the configured port.
    /// Does NOT block — returns immediately after wiring completes.
    virtual void Start() = 0;

    /// Signals graceful shutdown: closes transport, stops the event loop.
    virtual void Stop() noexcept = 0;
};

struct ServerDependencies {
    std::unique_ptr<asio::io_context> io_ctx;

    std::unique_ptr<IIdGenerator> id_generator;

    std::unique_ptr<IClientRepository> client_repository;

    std::unique_ptr<ITransport> transport;

    std::unique_ptr<IPacketRouter> router;

    std::unique_ptr<IConnectHandler> connect_handler;

    std::unique_ptr<IRelayHandler> relay_handler;
};

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

    std::unique_ptr<IClientRepository> client_repository_;

    std::unique_ptr<ITransport> transport_;

    std::unique_ptr<IPacketRouter> router_;

    std::unique_ptr<IConnectHandler> connect_handler_;

    std::unique_ptr<IRelayHandler> relay_handler_;

    uint16_t port_;

    static inline ILogger* logger_ = new ConsoleLogger();
};
