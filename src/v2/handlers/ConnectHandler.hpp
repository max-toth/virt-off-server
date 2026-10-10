#pragma once

#include "core/ConsoleLogger.hpp"
#include "net/ITransport.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "v2/core/IIdGenerator.hpp"
#include "v2/handlers/IConnectHandler.hpp"

namespace v2 {

/// Initial-connection handler.
///
/// Assigns a string id via IIdGenerator, stores a full UserInfo record in
/// IUserRepository, and replies with the assigned id.
class ConnectHandler final : public IConnectHandler {
public:
    ConnectHandler(IIdGenerator& id_gen, IUserRepository& users, ITransport& transport);

    void handle(const udp::endpoint& client_endpoint) override;

private:
    IIdGenerator& id_gen_;

    IUserRepository& users_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2
