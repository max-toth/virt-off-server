# virt-off-server Design Spec

## Goal

Port the Unity voice-chat relay server from C# (`NetManager` server side) to a standalone C++ project, then deploy on Yandex Compute Cloud.

## Protocol (copy from Unity client)

| Byte 0  | Payload          | Direction          | Action                              |
|---------|------------------|--------------------|-------------------------------------|
| `0xFF`  | —                | Client → Server    | Connect request                     |
| `0xFE`  | uint64 clientId  | Server → Client    | Assign ID (8 bytes)                 |
| voice   | Opus data        | Client → Server    | Voice packet                        |
| voice   | Opus data        | Server → All⁻¹     | Voice relay (broadcast, exclude src) |

- Server listens on UDP port **7777**.
- `0xFF` and `0xFE` are the only control messages — everything else is relayed as-is.
- Server does NOT inspect voice packet contents.

## Architecture

Single-threaded `asio::io_context` event loop. One UDP socket, non-blocking.

```
io_context.run()
  └─ socket.async_receive_from()
       └─ handle_packet(endpoint, data)
            ├─ 0xFF → assign ID, store client, send 0xFE response
            ├─ 0xFE → (should not happen on server — ignore)
            └─ other → relay to all stored clients except sender
```

## Project Structure (interface-first)

```
virt-off-server/
├── src/
│   ├── core/
│   │   ├── ILogger.hpp
│   │   └── IIdGenerator.hpp
│   ├── clients/
│   │   └── IClientRepository.hpp
│   ├── net/
│   │   ├── ITransport.hpp
│   │   └── IPacketRouter.hpp
│   ├── handlers/
│   │   ├── IConnectHandler.hpp
│   │   └── IRelayHandler.hpp
│   └── app/
│       └── Server.hpp         ← composition root
├── CMakeLists.txt
├── Dockerfile
├── deploy.sh
├── .clang-format
├── .gitignore
├── RULES.md
└── README.md
```

## Interface Design

### `core/ILogger.hpp`
```cpp
struct ILogger {
    virtual ~ILogger() = default;
    virtual void info(std::string_view msg) = 0;
    virtual void warn(std::string_view msg) = 0;
    virtual void error(std::string_view msg) = 0;
};
```

### `core/IIdGenerator.hpp`
```cpp
struct IIdGenerator {
    virtual ~IIdGenerator() = default;
    virtual uint64_t next() = 0;    // monotonic, thread-safe not required
};
```

### `clients/IClientRepository.hpp`
```cpp
struct IClientRepository {
    virtual ~IClientRepository() = default;
    virtual uint64_t add(asio::ip::udp::endpoint ep) = 0;
    virtual void remove(uint64_t id) = 0;
    virtual std::optional<uint64_t> find_by_endpoint(asio::ip::udp::endpoint ep) const = 0;
    virtual void for_each_except(uint64_t exclude_id, std::function<void(uint64_t, asio::ip::udp::endpoint)> fn) const = 0;
};
```

### `net/ITransport.hpp`
```cpp
struct ITransport {
    virtual ~ITransport() = default;
    virtual void start(asio::ip::udp::endpoint local) = 0;         // bind
    virtual void send_to(asio::ip::udp::endpoint dest, std::span<const uint8_t> data) = 0;
    virtual void receive(std::function<void(asio::ip::udp::endpoint, std::vector<uint8_t>)> on_packet) = 0;
};
```

### `net/IPacketRouter.hpp`
```cpp
struct IPacketRouter {
    virtual ~IPacketRouter() = default;
    // Register handler for first byte of packet
    virtual void register_handler(uint8_t type, std::function<void(asio::ip::udp::endpoint, std::span<const uint8_t>)> handler) = 0;
    virtual void route(asio::ip::udp::endpoint from, std::span<const uint8_t> data) = 0;
};
```

### `handlers/IConnectHandler.hpp`
```cpp
struct IConnectHandler {
    virtual ~IConnectHandler() = default;
    // Called when 0xFF packet received. Assigns ID, stores client, sends 0xFE response.
    virtual void handle(asio::ip::udp::endpoint from) = 0;
};
```

### `handlers/IRelayHandler.hpp`
```cpp
struct IRelayHandler {
    virtual ~IRelayHandler() = default;
    // Relays data to all connected clients except the sender.
    virtual void relay(asio::ip::udp::endpoint from, std::span<const uint8_t> data) = 0;
};
```

### `app/Server.hpp`
Composition root — receives all interfaces via constructor, wires them together, starts transport.

## Infrastructure

- **Docker:** multi-stage build (gcc:13 → alpine:3.19), static binary (`-static`).
- **Deploy:** `deploy.sh` with placeholder for YC Container Registry.
- **CI/CD:** manual for now — `docker build && docker push && yc compute instance update-container`.

## Constraints

- No runtime dependencies except libc.
- Single-threaded.
- No config file — port via `argv[1]` (default 7777).
- Must build on both macOS (development) and Linux (deploy).
