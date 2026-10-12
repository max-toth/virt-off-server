#pragma once

#include "v2/game/IAttackGate.hpp"
#include "v2/game/IAttackHandler.hpp"
#include "v2/game/IBroadcastHandler.hpp"
#include "v2/game/IManifestHandler.hpp"
#include "v2/game/IPickupHandler.hpp"
#include "v2/game/IPlayerState.hpp"
#include "v2/game/IPropRepository.hpp"
#include "v2/game/IPropStateHandler.hpp"
#include "v2/game/IReleaseHandler.hpp"
#include "v2/game/ISessionMonitor.hpp"

#include <memory>

namespace v2::game {

/// Orchestrator of game-logic services and handlers.
///
/// Owns all game-layer interfaces as unique_ptr. Provides accessors so
/// Server (or a test) can fetch individual services. Not yet wired into
/// the Server — built standalone via Builder for testing and future
/// integration.
///
/// Movable, non-copyable.
class GameOrchestrator {
public:
    // -- Accessors (raw pointers — orchestrator owns the lifetime) --

    IPlayerState& player_state();
    IPropRepository& prop_repository();
    IAttackGate& attack_gate();
    IBroadcastHandler& broadcast_handler();
    ISessionMonitor& session_monitor();
    IManifestHandler& manifest_handler();
    IPickupHandler& pickup_handler();
    IReleaseHandler& release_handler();
    IPropStateHandler& prop_state_handler();
    IAttackHandler& attack_handler();

    // -- Builder --

    class Builder {
    public:
        Builder& with_player_state(std::unique_ptr<IPlayerState> impl);
        Builder& with_prop_repository(std::unique_ptr<IPropRepository> impl);
        Builder& with_attack_gate(std::unique_ptr<IAttackGate> impl);
        Builder& with_broadcast_handler(std::unique_ptr<IBroadcastHandler> impl);
        Builder& with_session_monitor(std::unique_ptr<ISessionMonitor> impl);
        Builder& with_manifest_handler(std::unique_ptr<IManifestHandler> impl);
        Builder& with_pickup_handler(std::unique_ptr<IPickupHandler> impl);
        Builder& with_release_handler(std::unique_ptr<IReleaseHandler> impl);
        Builder& with_prop_state_handler(std::unique_ptr<IPropStateHandler> impl);
        Builder& with_attack_handler(std::unique_ptr<IAttackHandler> impl);

        [[nodiscard]] GameOrchestrator build();

    private:
        /// Player transform registry.
        std::unique_ptr<IPlayerState> player_state_;

        /// Prop ownership and state arbiter.
        std::unique_ptr<IPropRepository> prop_repository_;

        /// Attack cooldown + distance validator.
        std::unique_ptr<IAttackGate> attack_gate_;

        /// Sends to all users including author.
        std::unique_ptr<IBroadcastHandler> broadcast_handler_;

        /// Session timeout / sweep.
        std::unique_ptr<ISessionMonitor> session_monitor_;

        /// 0x06 manifest handler.
        std::unique_ptr<IManifestHandler> manifest_handler_;

        /// 0x07 pickup handler.
        std::unique_ptr<IPickupHandler> pickup_handler_;

        /// 0x08 release handler.
        std::unique_ptr<IReleaseHandler> release_handler_;

        /// 0x04 prop state handler.
        std::unique_ptr<IPropStateHandler> prop_state_handler_;

        /// 0x03 attack handler.
        std::unique_ptr<IAttackHandler> attack_handler_;
    };

    GameOrchestrator(GameOrchestrator&&) = default;
    GameOrchestrator& operator=(GameOrchestrator&&) = default;
    GameOrchestrator(const GameOrchestrator&) = delete;
    GameOrchestrator& operator=(const GameOrchestrator&) = delete;

private:
    friend class Builder;

    explicit GameOrchestrator(std::unique_ptr<IPlayerState> player_state,
                              std::unique_ptr<IPropRepository> prop_repository,
                              std::unique_ptr<IAttackGate> attack_gate,
                              std::unique_ptr<IBroadcastHandler> broadcast_handler,
                              std::unique_ptr<ISessionMonitor> session_monitor,
                              std::unique_ptr<IManifestHandler> manifest_handler,
                              std::unique_ptr<IPickupHandler> pickup_handler,
                              std::unique_ptr<IReleaseHandler> release_handler,
                              std::unique_ptr<IPropStateHandler> prop_state_handler,
                              std::unique_ptr<IAttackHandler> attack_handler);

    /// Player transform registry.
    std::unique_ptr<IPlayerState> player_state_;

    /// Prop ownership and state arbiter.
    std::unique_ptr<IPropRepository> prop_repository_;

    /// Attack cooldown + distance validator.
    std::unique_ptr<IAttackGate> attack_gate_;

    /// Sends to all users including author.
    std::unique_ptr<IBroadcastHandler> broadcast_handler_;

    /// Session timeout / sweep.
    std::unique_ptr<ISessionMonitor> session_monitor_;

    /// 0x06 manifest handler.
    std::unique_ptr<IManifestHandler> manifest_handler_;

    /// 0x07 pickup handler.
    std::unique_ptr<IPickupHandler> pickup_handler_;

    /// 0x08 release handler.
    std::unique_ptr<IReleaseHandler> release_handler_;

    /// 0x04 prop state handler.
    std::unique_ptr<IPropStateHandler> prop_state_handler_;

    /// 0x03 attack handler.
    std::unique_ptr<IAttackHandler> attack_handler_;
};

}  // namespace v2::game
