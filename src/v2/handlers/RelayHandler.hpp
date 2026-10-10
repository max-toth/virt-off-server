#pragma once

#include "core/ConsoleLogger.hpp"
#include "net/ITransport.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "v2/handlers/IRelayHandler.hpp"

namespace v2 {

/// Relays voice/position packets between users.
///
/// Resolves the sender via IUserRepository and broadcasts to everyone else.
class RelayHandler final : public IRelayHandler {
public:
    RelayHandler(IUserRepository& users, ITransport& transport);

    void relay(uint8_t type, const udp::endpoint& sender, std::span<const uint8_t> data) override;

private:
    IUserRepository& users_;

    ITransport& transport_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2
