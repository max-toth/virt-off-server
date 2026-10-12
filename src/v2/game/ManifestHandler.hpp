#pragma once

#include "v2/game/IManifestHandler.hpp"

#include "v2/game/IBroadcastHandler.hpp"
#include "v2/game/IPropRepository.hpp"
#include "v2/clients/IUserRepository.hpp"
#include "core/ConsoleLogger.hpp"

namespace v2::game {

/// Handles 0x06 MANIFEST: parses manifest payload, registers props,
/// replies with MANIFEST_ACK.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class ManifestHandler final : public IManifestHandler {
public:
    ManifestHandler(IUserRepository& users,
                    IPropRepository& props,
                    IBroadcastHandler& broadcast);

    void handle(const udp::endpoint& from, std::span<const uint8_t> payload) override;

private:
    IUserRepository& users_;

    IPropRepository& props_;

    IBroadcastHandler& broadcast_;

    static inline ILogger* logger_ = new ConsoleLogger();
};

}  // namespace v2::game
