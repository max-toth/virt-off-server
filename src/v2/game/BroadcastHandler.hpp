#pragma once

#include "v2/game/IBroadcastHandler.hpp"

#include "v2/clients/IUserRepository.hpp"
#include "net/ITransport.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace v2::game {

/// Broadcasts packets to all users (including author) via ITransport.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class BroadcastHandler final : public IBroadcastHandler {
public:
    BroadcastHandler(IUserRepository& users, ITransport& transport);

    void broadcast(uint8_t type, std::span<const uint8_t> data) override;

    void send_to(const std::string& id, uint8_t type, std::span<const uint8_t> data) override;

private:
    /// Prepends `type` to `data` and returns the assembled datagram.
    static std::vector<uint8_t> assemble_datagram(uint8_t type, std::span<const uint8_t> data);

    IUserRepository& users_;

    ITransport& transport_;
};

}  // namespace v2::game
