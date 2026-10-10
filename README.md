# virt-off-server

C++ UDP relay server for the Virt-Off virtual office voice chat. Receives Opus-encoded voice packets from Unity clients and relays them to all other connected clients.

## Architecture

```
┌──────────────┐  UDP :7777  ┌──────────────────┐
│  Unity Client │ ◄─────────► │  virt-off-server  │
│  (C#)          │             │  (C++ / asio)      │
└──────────────┘             └──────────────────┘
```

Single-threaded `asio::io_context` event loop. No external dependencies beyond asio (header-only).

## Protocol

| Byte 0 | Payload          | Direction          | Description              |
|--------|------------------|--------------------|--------------------------|
| `0xFF` | —                | Client → Server    | Connect request          |
| `0xFE` | uint64 clientId  | Server → Client    | ID assignment (8 bytes)  |
| any    | Opus data        | Client → Server    | Voice packet             |
| any    | Opus data        | Server → All⁻¹    | Voice relay (excl. src)  |

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/virt-off-server           # port 7777
./build/virt-off-server 8888      # custom port
```

Stop with `Ctrl+C`.

## Docker

```bash
docker build -t virt-off-server .
docker run --rm -p 7777:7777/udp virt-off-server
```

## Deploy to Yandex Cloud

```bash
./deploy.sh
```

Prompts for Container Registry ID on first run.

## Project Structure

```
src/
├── core/              — ILogger, IIdGenerator, ConsoleLogger, MonotonicIdGenerator
├── clients/           — IClientRepository, ClientRepository
├── net/               — ITransport, IPacketRouter, UdpTransport, PacketRouter
├── handlers/          — IConnectHandler, IRelayHandler, ConnectHandler, RelayHandler
└── app/               — IServer, Server (composition root)
```

## Development Rules

See `RULES.md` — interface-first design, SRP, composition over implementation.
