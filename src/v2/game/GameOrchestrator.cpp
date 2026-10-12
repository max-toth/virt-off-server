#include "v2/game/GameOrchestrator.hpp"

namespace v2::game {

// -- Accessors --

IPlayerState& GameOrchestrator::player_state() { return *player_state_; }
IPropRepository& GameOrchestrator::prop_repository() { return *prop_repository_; }
IAttackGate& GameOrchestrator::attack_gate() { return *attack_gate_; }
IBroadcastHandler& GameOrchestrator::broadcast_handler() { return *broadcast_handler_; }
ISessionMonitor& GameOrchestrator::session_monitor() { return *session_monitor_; }
IManifestHandler& GameOrchestrator::manifest_handler() { return *manifest_handler_; }
IPickupHandler& GameOrchestrator::pickup_handler() { return *pickup_handler_; }
IReleaseHandler& GameOrchestrator::release_handler() { return *release_handler_; }
IPropStateHandler& GameOrchestrator::prop_state_handler() { return *prop_state_handler_; }
IAttackHandler& GameOrchestrator::attack_handler() { return *attack_handler_; }

// -- Builder --

GameOrchestrator::Builder& GameOrchestrator::Builder::with_player_state(std::unique_ptr<IPlayerState> impl) {
    player_state_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_prop_repository(std::unique_ptr<IPropRepository> impl) {
    prop_repository_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_attack_gate(std::unique_ptr<IAttackGate> impl) {
    attack_gate_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_broadcast_handler(std::unique_ptr<IBroadcastHandler> impl) {
    broadcast_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_session_monitor(std::unique_ptr<ISessionMonitor> impl) {
    session_monitor_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_manifest_handler(std::unique_ptr<IManifestHandler> impl) {
    manifest_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_pickup_handler(std::unique_ptr<IPickupHandler> impl) {
    pickup_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_release_handler(std::unique_ptr<IReleaseHandler> impl) {
    release_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_prop_state_handler(std::unique_ptr<IPropStateHandler> impl) {
    prop_state_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator::Builder& GameOrchestrator::Builder::with_attack_handler(std::unique_ptr<IAttackHandler> impl) {
    attack_handler_ = std::move(impl);
    return *this;
}

GameOrchestrator GameOrchestrator::Builder::build() {
    return GameOrchestrator(
        std::move(player_state_),
        std::move(prop_repository_),
        std::move(attack_gate_),
        std::move(broadcast_handler_),
        std::move(session_monitor_),
        std::move(manifest_handler_),
        std::move(pickup_handler_),
        std::move(release_handler_),
        std::move(prop_state_handler_),
        std::move(attack_handler_));
}

// -- Private constructor --

GameOrchestrator::GameOrchestrator(std::unique_ptr<IPlayerState> player_state,
                                   std::unique_ptr<IPropRepository> prop_repository,
                                   std::unique_ptr<IAttackGate> attack_gate,
                                   std::unique_ptr<IBroadcastHandler> broadcast_handler,
                                   std::unique_ptr<ISessionMonitor> session_monitor,
                                   std::unique_ptr<IManifestHandler> manifest_handler,
                                   std::unique_ptr<IPickupHandler> pickup_handler,
                                   std::unique_ptr<IReleaseHandler> release_handler,
                                   std::unique_ptr<IPropStateHandler> prop_state_handler,
                                   std::unique_ptr<IAttackHandler> attack_handler)
    : player_state_(std::move(player_state))
    , prop_repository_(std::move(prop_repository))
    , attack_gate_(std::move(attack_gate))
    , broadcast_handler_(std::move(broadcast_handler))
    , session_monitor_(std::move(session_monitor))
    , manifest_handler_(std::move(manifest_handler))
    , pickup_handler_(std::move(pickup_handler))
    , release_handler_(std::move(release_handler))
    , prop_state_handler_(std::move(prop_state_handler))
    , attack_handler_(std::move(attack_handler)) {}

}  // namespace v2::game
