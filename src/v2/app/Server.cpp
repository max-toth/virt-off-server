#include "v2/app/Server.hpp"

#include "net/ProtocolConstants.hpp"
#include "v2/game/GameConstants.hpp"
#include "v2/game/WireSizes.hpp"

#include <chrono>
#include <cstring>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace v2 {

Server::Server(ServerDependencies deps, uint16_t port)
    : io_ctx_(std::move(deps.io_ctx))
    , id_generator_(std::move(deps.id_generator))
    , user_repository_(std::move(deps.user_repository))
    , transport_(std::move(deps.transport))
    , router_(std::move(deps.router))
    , connect_handler_(std::move(deps.connect_handler))
    , relay_handler_(std::move(deps.relay_handler))
    , game_orchestrator_(std::move(deps.game_orchestrator))
    , port_(port) {
    logger_->info("Server (v2) object created, port=" + std::to_string(port_));
}

Server::~Server() {
    Stop();
}

void Server::Start() {
    logger_->info("Starting server (v2) on port " + std::to_string(port_));

    auto local = udp::endpoint(asio::ip::udp::v4(), port_);
    transport_->start(local);

    router_->register_handler(
        protocol::MSG_CONNECT,
        [this](const udp::endpoint& from, std::span<const uint8_t> /*payload*/) { connect_handler_->handle(from); });

    router_->register_handler(protocol::MSG_ASSIGN_ID,
                              [](const udp::endpoint& /*from*/, std::span<const uint8_t> /*payload*/) {});

    router_->register_handler(protocol::MSG_VOICE, [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
        relay_handler_->relay(protocol::MSG_VOICE, from, payload);
    });

    router_->register_handler(protocol::MSG_POSITION,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  relay_handler_->relay(protocol::MSG_POSITION, from, payload);
                              });

    // -- Game handlers --

    router_->register_handler(protocol::MSG_ATTACK,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  game_orchestrator_->attack_handler().handle(from, payload);
                              });

    router_->register_handler(protocol::MSG_PROP_STATE,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  game_orchestrator_->prop_state_handler().handle(from, payload);
                              });

    router_->register_handler(protocol::MSG_SOUND,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  relay_handler_->relay(protocol::MSG_SOUND, from, payload);
                              });

    router_->register_handler(protocol::MSG_MANIFEST,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  game_orchestrator_->manifest_handler().handle(from, payload);
                              });

    router_->register_handler(protocol::MSG_PICKUP_REQ,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  game_orchestrator_->pickup_handler().handle(from, payload);
                              });

    router_->register_handler(protocol::MSG_PROP_RELEASE,
                              [this](const udp::endpoint& from, std::span<const uint8_t> payload) {
                                  game_orchestrator_->release_handler().handle(from, payload);
                              });

    router_->register_handler(protocol::MSG_DISCONNECT,
                              [this](const udp::endpoint& from, std::span<const uint8_t> /*payload*/) {
                                  const auto sender_id = user_repository_->find_id_by_endpoint(from);
                                  if (!sender_id.has_value()) {
                                      return;
                                  }
                                  logger_->info("[Server] DISCONNECT " + *sender_id);
                                  // Graceful disconnect: release props and remove user.
                                  game_orchestrator_->prop_repository().release_all_of(*sender_id);
                                  game_orchestrator_->prop_repository().for_each(
                                      [this](const game::PropRecord& rec) {
                                          if (rec.owner_id.empty()) {
                                              const size_t payload_size = game::kU32Bytes + game::kStrLenBytes + game::kVec3Bytes + game::kVec3Bytes + game::kFlagsBytes;
                                              std::vector<uint8_t> payload(payload_size, 0);
                                              std::memcpy(payload.data(), &rec.prop_id, game::kU32Bytes);
                                              payload.back() = game::kPropFlagSettled;
                                              game_orchestrator_->broadcast_handler().broadcast(
                                                  protocol::MSG_PROP_RELEASE_EVENT, payload);
                                          }
                                      });
                                  user_repository_->remove(*sender_id);
                              });

    // Receive wrapper: mark_active + player_state update for POSITION.
    transport_->set_receive_callback(
        [this](udp::endpoint from, std::vector<uint8_t> data) {
            if (!data.empty()) {
                const auto sender_id = user_repository_->find_id_by_endpoint(from);
                if (sender_id.has_value()) {
                    game_orchestrator_->session_monitor().mark_active(*sender_id);

                    // For POSITION packets, update player state before routing.
                    // POSITION payload: [id:str][pos:12][rot:16][vel:12]
                    if (data[0] == protocol::MSG_POSITION) {
                        const uint8_t id_len = data[1];
                        constexpr size_t pos_payload_min = game::kStrLenBytes + game::kVec3Bytes + game::kQuatBytes + game::kVec3Bytes;
                        if (data.size() >= 1 + game::kStrLenBytes + id_len + pos_payload_min) {
                            const size_t pos_off = 1 + game::kStrLenBytes + id_len;
                            game::PlayerTransform transform;
                            std::memcpy(transform.position,  data.data() + pos_off,                              game::kVec3Bytes);
                            std::memcpy(transform.rotation,  data.data() + pos_off + game::kVec3Bytes,           game::kQuatBytes);
                            std::memcpy(transform.velocity,  data.data() + pos_off + game::kVec3Bytes + game::kQuatBytes, game::kVec3Bytes);
                            game_orchestrator_->player_state().update(*sender_id, transform);
                        }
                    }
                }
            }
            router_->route(from, data);
        });

    // Tick callback: sweep idle sessions.
    transport_->set_tick_callback(
        [this]() {
            const auto before = user_repository_->size();
            game_orchestrator_->session_monitor().sweep(std::chrono::system_clock::now());
            const auto after = user_repository_->size();
            if (before != after) {
                logger_->info("[Server] sweep: " + std::to_string(before - after) + " idle sessions dropped");
            }
        },
        game::kSweepIntervalMs);

    logger_->info("Server (v2) is running");
}

void Server::Stop() noexcept {
    if (transport_) {
        transport_->stop();
    }

    if (io_ctx_ && !io_ctx_->stopped()) {
        io_ctx_->stop();
    }
}

}  // namespace v2
