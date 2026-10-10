#pragma once

#include "clients/IClientRepository.hpp"
#include "core/ConsoleLogger.hpp"
#include "handlers/IRelayHandler.hpp"
#include "net/ITransport.hpp"

#include <cstdint>
#include <cstring>
#include <vector>

class RelayHandler final : public IRelayHandler {
public:
    RelayHandler(IClientRepository& clients, ITransport& transport);

    void relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) override;

private:
    IClientRepository& clients_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};
