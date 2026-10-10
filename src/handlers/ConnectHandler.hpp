#pragma once

#include "handlers/IConnectHandler.hpp"
#include "core/ConsoleLogger.hpp"

#include "core/IIdGenerator.hpp"
#include "clients/IClientRepository.hpp"
#include "net/ITransport.hpp"

class ConnectHandler final : public IConnectHandler {
public:
    ConnectHandler(IIdGenerator& id_gen,
                   IClientRepository& clients,
                   ITransport& transport)
        : id_gen_(id_gen), clients_(clients), transport_(transport)
    {
        logger_->info("ConnectHandler created");
    }

    void handle(const udp::endpoint& client_endpoint) override;

private:
    IIdGenerator& id_gen_;

    IClientRepository& clients_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};
