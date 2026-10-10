#pragma once

#include "clients/IUserRepositoryV2.hpp"
#include "core/ConsoleLogger.hpp"
#include "core/IIdGeneratorV2.hpp"
#include "handlers/IConnectHandlerV2.hpp"
#include "net/ITransport.hpp"

/// V2 initial-connection handler.
///
/// Assigns a string id via IIdGeneratorV2, stores a full UserInfo record in
/// IUserRepositoryV2, and replies with the assigned id.
class ConnectHandlerV2 final : public IConnectHandlerV2 {
public:
    ConnectHandlerV2(IIdGeneratorV2& id_gen, IUserRepositoryV2& users, ITransport& transport);

    void handle(const udp::endpoint& client_endpoint) override;

private:
    IIdGeneratorV2& id_gen_;

    IUserRepositoryV2& users_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};
