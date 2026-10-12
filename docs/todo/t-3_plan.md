# t-3 Этап 1 — Интерфейсы, константы, GameOrchestrator

## Стиль кода

- Константы разделять пустыми строками.
- Приватные поля классов документировать и разделять пустыми строками.

## Модифицируемые файлы

### 1. `src/net/ProtocolConstants.hpp`

Добавить после `MSG_POSITION`, перед закрывающим `}`:

```cpp
/// 0x03 — C→S: attack request. Server validates (cooldown + distance) then relays to target.
inline constexpr uint8_t MSG_ATTACK       = 0x03;

/// 0x04 — C→S → S→all: prop transform state from owner.
inline constexpr uint8_t MSG_PROP_STATE   = 0x04;

/// 0x05 — C→S → S→all: sound event relay (includes author).
inline constexpr uint8_t MSG_SOUND        = 0x05;

/// 0x06 — C→S: scene prop manifest (key + halfExtents + mass × count).
inline constexpr uint8_t MSG_MANIFEST     = 0x06;

/// 0x07 — C→S: pickup request for a specific prop.
inline constexpr uint8_t MSG_PICKUP_REQ   = 0x07;

/// 0x08 — C→S: prop release / throw from holder.
inline constexpr uint8_t MSG_PROP_RELEASE = 0x08;

/// 0x09 — S→C: manifest ACK — authoritative propId mapping.
inline constexpr uint8_t MSG_MANIFEST_ACK = 0x09;

/// 0x0A — S→all: prop granted to a holder.
inline constexpr uint8_t MSG_PROP_GRANT   = 0x0A;

/// 0x0B — S→all: prop released (owner empty → FREE).
inline constexpr uint8_t MSG_PROP_RELEASE_EVENT = 0x0B;

/// 0x0C — S→C: full prop ownership snapshot on join.
inline constexpr uint8_t MSG_PROP_SNAPSHOT = 0x0C;

/// 0x0D — C→S: graceful disconnect (owner stops sending).
inline constexpr uint8_t MSG_DISCONNECT   = 0x0D;
```

Обратная совместимость: V1 не использует `0x03..0x0D`, добавление
безопасно.

---

### 2. `src/net/ITransport.hpp`

Добавить в `struct ITransport`:

```cpp
/// Registers a periodic timer callback on the transport strand.
/// `callback` is invoked on the strand every `interval_ms` milliseconds.
/// The timer is created lazily on the first call to `start()`.
virtual void set_tick_callback(std::function<void()> callback, unsigned int interval_ms) = 0;
```

---

### 3. `src/v2/clients/IUserRepository.hpp`

Добавить в `struct IUserRepository`:

```cpp
/// Invokes `fn` for every user (unlike for_each_except — no exclusion).
virtual void for_each(const std::function<void(const UserInfo&)>& fn) const = 0;

/// Removes all users whose `last_seen` is older than `now - timeout`.
virtual void remove_idle(std::chrono::system_clock::time_point now,
                          std::chrono::seconds timeout) = 0;
```

---

## Новые файлы: data-структуры (`src/v2/game/`)

### 4. `src/v2/game/PropDescriptor.hpp`

```cpp
#pragma once

#include <cstdint>
#include <string>

namespace v2::game {

/// Descriptor of a single prop sent by the client in the MANIFEST message.
struct PropDescriptor {
    std::string key;         ///< Identifier (e.g. hierarchy path), used for matching.
    float half_extents[3];   ///< x, y, z half-extents for spatial validation.
    float mass;              ///< Prop mass in kg (affects throw impulse).
};

}  // namespace v2::game
```

### 5. `src/v2/game/PropRecord.hpp`

```cpp
#pragma once

#include <cstdint>
#include <string>

namespace v2::game {

/// Server-side canonical record for a single prop.
///
/// Owner is authoritative — only the holder may send PROP_STATE.
/// `held == false` means the prop is resting at its last-released position.
struct PropRecord {
    uint32_t prop_id;        ///< Canonical id assigned by server in MANIFEST_ACK.
    std::string key;         ///< Matches PropDescriptor::key from manifest.
    std::string owner_id;    ///< Empty string means free (no owner).
    bool held;               ///< True while the owner actively holds the prop.
};

}  // namespace v2::game
```

### 6. `src/v2/game/PlayerTransform.hpp`

```cpp
#pragma once

#include <cstdint>

namespace v2::game {

/// Snapshot of a player's spatial state for attack validation.
struct PlayerTransform {
    float position[3];   ///< World-space position (x, y, z).
    float rotation[4];   ///< Quaternion (x, y, z, w).
    float velocity[3];   ///< Linear velocity (x, y, z).
};

}  // namespace v2::game
```

---

## Новые файлы: service-интерфейсы (`src/v2/game/`)

### 7. `src/v2/game/IPlayerState.hpp`

```cpp
#pragma once

#include "v2/game/PlayerTransform.hpp"

#include <optional>
#include <string>

namespace v2::game {

/// Tracks the latest transform of each player.
///
/// Used by IAttackGate for distance validation. Writing happens on
/// every POSITION packet; reading on every ATTACK.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IPlayerState {
    virtual ~IPlayerState() = default;

    /// Stores or replaces the transform for `id`.
    virtual void update(const std::string& id, const PlayerTransform& transform) = 0;

    /// Returns the latest known transform for `id`, or nullopt if unknown.
    virtual std::optional<PlayerTransform> get(const std::string& id) const = 0;

    /// Removes the transform for `id` (used on disconnect/timeout).
    virtual void erase(const std::string& id) = 0;
};

}  // namespace v2::game
```

### 8. `src/v2/game/IPropRepository.hpp`

```cpp
#pragma once

#include "v2/game/PropDescriptor.hpp"
#include "v2/game/PropRecord.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace v2::game {

/// Canonical prop registry and ownership arbiter.
///
/// One record per prop. On grant the server verifies the prop is free;
/// on release it clears the owner. All changes are authoritative.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IPropRepository {
    virtual ~IPropRepository() = default;

    /// Registers props from a client manifest. Existing props with the
    /// same key are NOT replaced (first manifest wins). Assigns prop_ids.
    virtual void register_manifest(const std::vector<PropDescriptor>& descriptors) = 0;

    /// Atomically grants `prop_id` to `holder_id` if the prop is free.
    /// Returns true on success, false if already held.
    virtual bool try_grant(uint32_t prop_id, const std::string& holder_id) = 0;

    /// Releases `prop_id` — clears owner, sets held=false.
    /// Returns the record before release, or nullopt if unknown.
    virtual std::optional<PropRecord> release(uint32_t prop_id) = 0;

    /// Releases every prop owned by `owner_id` (used on disconnect).
    virtual void release_all_of(const std::string& owner_id) = 0;

    /// Returns a copy of the record for `prop_id`, or nullopt.
    virtual std::optional<PropRecord> get(uint32_t prop_id) const = 0;

    /// Invokes `fn` for every registered prop.
    virtual void for_each(const std::function<void(const PropRecord&)>& fn) const = 0;

    /// Returns the total number of registered props.
    virtual size_t size() const = 0;
};

}  // namespace v2::game
```

### 9. `src/v2/game/IAttackGate.hpp`

```cpp
#pragma once

#include <chrono>
#include <string>

namespace v2::game {

/// Validates whether an attack between two players is allowed.
///
/// Currently holds two checks:
///   1. Cooldown — attacker cannot hit the same target too frequently.
///   2. Distance — attacker must be within range of target.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IAttackGate {
    virtual ~IAttackGate() = default;

    /// Returns true if `attacker_id` may hit `target_id` at `now`.
    virtual bool allow_attack(const std::string& attacker_id,
                               const std::string& target_id,
                               std::chrono::system_clock::time_point now) = 0;
};

}  // namespace v2::game
```

### 10. `src/v2/game/IBroadcastHandler.hpp`

```cpp
#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace v2::game {

/// Sends packets to all connected users (INCLUDING the author).
///
/// Unlike IRelayHandler which excludes the sender, broadcast includes
/// everyone — used for PROP_GRANT, PROP_RELEASE_EVENT, MANIFEST_ACK.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IBroadcastHandler {
    virtual ~IBroadcastHandler() = default;

    /// Sends `data` (without type byte) to ALL users, prepending `type`.
    virtual void broadcast(uint8_t type, std::span<const uint8_t> data) = 0;

    /// Sends `data` (without type byte) to a single user by `id`.
    virtual void send_to(const std::string& id, uint8_t type, std::span<const uint8_t> data) = 0;
};

}  // namespace v2::game
```

### 11. `src/v2/game/ISessionMonitor.hpp`

```cpp
#pragma once

#include <chrono>
#include <string>

namespace v2::game {

/// Tracks client liveness and performs session timeout / cleanup.
///
/// mark_active is called on every incoming packet (receive wrapper).
/// sweep is called periodically (tick callback) to evict idle sessions.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct ISessionMonitor {
    virtual ~ISessionMonitor() = default;

    /// Updates the last-seen timestamp for `id` to now.
    virtual void mark_active(const std::string& id) = 0;

    /// Finds and drops sessions idle longer than kSessionTimeout.
    /// Calls IUserRepository::remove_idle + IPropRepository::release_all_of
    /// + IBroadcastHandler::broadcast for each dropped session.
    virtual void sweep(std::chrono::system_clock::time_point now) = 0;
};

}  // namespace v2::game
```

---

## Новые файлы: handler-интерфейсы (`src/v2/game/`)

### 12–16. Handler интерфейсы

Каждый — чистый интерфейс с единственным методом `handle`,
единообразный паттерн.

**`src/v2/game/IManifestHandler.hpp`**:

```cpp
#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

/// Handles 0x06 MANIFEST from a client.
/// Parses prop descriptors, calls IPropRepository::register_manifest,
/// replies with MANIFEST_ACK containing assigned prop IDs.
struct IManifestHandler {
    virtual ~IManifestHandler() = default;
    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
```

**`src/v2/game/IPickupHandler.hpp`**:

```cpp
#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

/// Handles 0x07 PICKUP_REQ from a client.
/// Validates distance, calls IPropRepository::try_grant,
/// broadcasts PROP_GRANT on success.
struct IPickupHandler {
    virtual ~IPickupHandler() = default;
    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
```

**`src/v2/game/IReleaseHandler.hpp`**:

```cpp
#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

/// Handles 0x08 PROP_RELEASE from a client.
/// Validates ownership, calls IPropRepository::release,
/// broadcasts PROP_RELEASE_EVENT.
struct IReleaseHandler {
    virtual ~IReleaseHandler() = default;
    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
```

**`src/v2/game/IPropStateHandler.hpp`**:

```cpp
#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

/// Handles 0x04 PROP_STATE from a client.
/// Validates ownership (only owner may send state),
/// broadcasts to all (including author).
struct IPropStateHandler {
    virtual ~IPropStateHandler() = default;
    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
```

**`src/v2/game/IAttackHandler.hpp`**:

```cpp
#pragma once

#include <asio/ip/udp.hpp>
#include <cstdint>
#include <span>

namespace v2::game {

/// Handles 0x03 ATTACK from a client.
/// Validates via IAttackGate, relays to target on success.
struct IAttackHandler {
    virtual ~IAttackHandler() = default;
    virtual void handle(const udp::endpoint& from, std::span<const uint8_t> payload) = 0;
};

}  // namespace v2::game
```

---

## 17. `src/v2/game/GameOrchestrator.hpp`

```cpp
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

    IPlayerState&       player_state();
    IPropRepository&    prop_repository();
    IAttackGate&        attack_gate();
    IBroadcastHandler&  broadcast_handler();
    ISessionMonitor&    session_monitor();
    IManifestHandler&   manifest_handler();
    IPickupHandler&     pickup_handler();
    IReleaseHandler&    release_handler();
    IPropStateHandler&  prop_state_handler();
    IAttackHandler&     attack_handler();

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
        /// Player transform registry (not owned).
        std::unique_ptr<IPlayerState>      player_state_;

        /// Prop ownership and state arbiter (not owned).
        std::unique_ptr<IPropRepository>   prop_repository_;

        /// Attack cooldown + distance validator (not owned).
        std::unique_ptr<IAttackGate>       attack_gate_;

        /// Sends to all users including author (not owned).
        std::unique_ptr<IBroadcastHandler> broadcast_handler_;

        /// Session timeout / sweep (not owned).
        std::unique_ptr<ISessionMonitor>   session_monitor_;

        /// 0x06 manifest handler (not owned).
        std::unique_ptr<IManifestHandler>  manifest_handler_;

        /// 0x07 pickup handler (not owned).
        std::unique_ptr<IPickupHandler>    pickup_handler_;

        /// 0x08 release handler (not owned).
        std::unique_ptr<IReleaseHandler>   release_handler_;

        /// 0x04 prop state handler (not owned).
        std::unique_ptr<IPropStateHandler> prop_state_handler_;

        /// 0x03 attack handler (not owned).
        std::unique_ptr<IAttackHandler>    attack_handler_;
    };

    GameOrchestrator(GameOrchestrator&&) = default;
    GameOrchestrator& operator=(GameOrchestrator&&) = default;
    GameOrchestrator(const GameOrchestrator&) = delete;
    GameOrchestrator& operator=(const GameOrchestrator&) = delete;

private:
    friend class Builder;

    explicit GameOrchestrator(std::unique_ptr<IPlayerState>      player_state,
                              std::unique_ptr<IPropRepository>   prop_repository,
                              std::unique_ptr<IAttackGate>       attack_gate,
                              std::unique_ptr<IBroadcastHandler> broadcast_handler,
                              std::unique_ptr<ISessionMonitor>   session_monitor,
                              std::unique_ptr<IManifestHandler>  manifest_handler,
                              std::unique_ptr<IPickupHandler>    pickup_handler,
                              std::unique_ptr<IReleaseHandler>   release_handler,
                              std::unique_ptr<IPropStateHandler> prop_state_handler,
                              std::unique_ptr<IAttackHandler>    attack_handler);

    /// Player transform registry.
    std::unique_ptr<IPlayerState>      player_state_;

    /// Prop ownership and state arbiter.
    std::unique_ptr<IPropRepository>   prop_repository_;

    /// Attack cooldown + distance validator.
    std::unique_ptr<IAttackGate>       attack_gate_;

    /// Sends to all users including author.
    std::unique_ptr<IBroadcastHandler> broadcast_handler_;

    /// Session timeout / sweep.
    std::unique_ptr<ISessionMonitor>   session_monitor_;

    /// 0x06 manifest handler.
    std::unique_ptr<IManifestHandler>  manifest_handler_;

    /// 0x07 pickup handler.
    std::unique_ptr<IPickupHandler>    pickup_handler_;

    /// 0x08 release handler.
    std::unique_ptr<IReleaseHandler>   release_handler_;

    /// 0x04 prop state handler.
    std::unique_ptr<IPropStateHandler> prop_state_handler_;

    /// 0x03 attack handler.
    std::unique_ptr<IAttackHandler>    attack_handler_;
};

}  // namespace v2::game
```

---

## Итог

| # | Файл | Действие |
|---|------|----------|
| 1 | `src/net/ProtocolConstants.hpp` | +11 констант 0x03..0x0D |
| 2 | `src/net/ITransport.hpp` | +pure virtual `set_tick_callback` |
| 3 | `src/v2/clients/IUserRepository.hpp` | +pure virtual `for_each`, `remove_idle` |
| 4 | `src/v2/game/PropDescriptor.hpp` | новый data-структура |
| 5 | `src/v2/game/PropRecord.hpp` | новый data-структура |
| 6 | `src/v2/game/PlayerTransform.hpp` | новый data-структура |
| 7 | `src/v2/game/IPlayerState.hpp` | новый service-интерфейс |
| 8 | `src/v2/game/IPropRepository.hpp` | новый service-интерфейс |
| 9 | `src/v2/game/IAttackGate.hpp` | новый service-интерфейс |
| 10 | `src/v2/game/IBroadcastHandler.hpp` | новый service-интерфейс |
| 11 | `src/v2/game/ISessionMonitor.hpp` | новый service-интерфейс |
| 12 | `src/v2/game/IManifestHandler.hpp` | новый handler-интерфейс |
| 13 | `src/v2/game/IPickupHandler.hpp` | новый handler-интерфейс |
| 14 | `src/v2/game/IReleaseHandler.hpp` | новый handler-интерфейс |
| 15 | `src/v2/game/IPropStateHandler.hpp` | новый handler-интерфейс |
| 16 | `src/v2/game/IAttackHandler.hpp` | новый handler-интерфейс |
| 17 | `src/v2/game/GameOrchestrator.hpp` | новый Builder-сборщик |
