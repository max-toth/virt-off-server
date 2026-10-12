#include "v2/app/ServerBuilder.hpp"

#include "net/PacketRouter.hpp"
#include "net/UdpTransport.hpp"
#include "v2/app/Server.hpp"
#include "v2/clients/UserRepository.hpp"
#include "v2/core/TimestampIdGenerator.hpp"
#include "v2/game/AttackGate.hpp"
#include "v2/game/AttackHandler.hpp"
#include "v2/game/BroadcastHandler.hpp"
#include "v2/game/GameOrchestrator.hpp"
#include "v2/game/ManifestHandler.hpp"
#include "v2/game/PickupHandler.hpp"
#include "v2/game/PlayerState.hpp"
#include "v2/game/PropRepository.hpp"
#include "v2/game/PropStateHandler.hpp"
#include "v2/game/ReleaseHandler.hpp"
#include "v2/game/SessionMonitor.hpp"
#include "v2/handlers/ConnectHandler.hpp"
#include "v2/handlers/RelayHandler.hpp"

#include <memory>

namespace v2 {

ServerBuilder::ServerBuilder(asio::io_context& io_ctx, uint16_t port) : io_ctx_(io_ctx), port_(port) {}

std::unique_ptr<IServer> ServerBuilder::build() {
    auto id_gen = std::make_unique<TimestampIdGenerator>();
    auto users = std::make_unique<UserRepository>();
    auto transport = std::make_unique<UdpTransport>(io_ctx_);
    auto router = std::make_unique<PacketRouter>();
    auto connect = std::make_unique<ConnectHandler>(*id_gen, *users, *transport);
    auto relay = std::make_unique<RelayHandler>(*users, *transport);

    // Build the game orchestrator.
    namespace game = v2::game;
    auto player_state = std::make_unique<game::PlayerState>();
    auto prop_repo = std::make_unique<game::PropRepository>();
    auto attack_gate = std::make_unique<game::AttackGate>(*player_state);
    auto broadcast_handler = std::make_unique<game::BroadcastHandler>(*users, *transport);
    auto session_monitor = std::make_unique<game::SessionMonitor>(*users, *prop_repo, *broadcast_handler);

    auto manifest_handler = std::make_unique<game::ManifestHandler>(*users, *prop_repo, *broadcast_handler);
    auto pickup_handler = std::make_unique<game::PickupHandler>(*users, *prop_repo, *player_state, *broadcast_handler);
    auto release_handler = std::make_unique<game::ReleaseHandler>(*users, *prop_repo, *broadcast_handler);
    auto prop_state_handler = std::make_unique<game::PropStateHandler>(*users, *prop_repo, *broadcast_handler);
    auto attack_handler = std::make_unique<game::AttackHandler>(*users, *attack_gate, *broadcast_handler);

    auto orchestrator = std::make_unique<game::GameOrchestrator>(
        game::GameOrchestrator::Builder{}
            .with_player_state(std::move(player_state))
            .with_prop_repository(std::move(prop_repo))
            .with_attack_gate(std::move(attack_gate))
            .with_broadcast_handler(std::move(broadcast_handler))
            .with_session_monitor(std::move(session_monitor))
            .with_manifest_handler(std::move(manifest_handler))
            .with_pickup_handler(std::move(pickup_handler))
            .with_release_handler(std::move(release_handler))
            .with_prop_state_handler(std::move(prop_state_handler))
            .with_attack_handler(std::move(attack_handler))
            .build());

    ServerDependencies deps{
        .io_ctx = nullptr,
        .id_generator = std::move(id_gen),
        .user_repository = std::move(users),
        .transport = std::move(transport),
        .router = std::move(router),
        .connect_handler = std::move(connect),
        .relay_handler = std::move(relay),
        .game_orchestrator = std::move(orchestrator),
    };

    return std::make_unique<Server>(std::move(deps), port_);
}

}  // namespace v2
