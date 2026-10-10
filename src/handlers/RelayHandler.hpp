#pragma once

#include "handlers/IRelayHandler.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

#include "core/ConsoleLogger.hpp"
#include "clients/IClientRepository.hpp"
#include "net/ITransport.hpp"

class RelayHandler final : public IRelayHandler {
public:
    RelayHandler(IClientRepository& clients, ITransport& transport);

    void relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) override;

private:
    IClientRepository& clients_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};
