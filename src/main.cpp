/// Application entry point.
///
/// Usage: virt-off-server [port] [threads]
///   port    — UDP port to listen on (default: 7777)
///   threads — number of io_context worker threads (default: hardware concurrency)

#include "app/Server.hpp"
#include "clients/ClientRepository.hpp"
#include "core/MonotonicIdGenerator.hpp"
#include "handlers/ConnectHandler.hpp"
#include "handlers/RelayHandler.hpp"
#include "net/PacketRouter.hpp"
#include "net/UdpTransport.hpp"

#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
    uint16_t port = 7777;
    if (argc > 1) {
        int p = std::atoi(argv[1]);
        if (p <= 0 || p > 65535) {
            std::cerr << "Invalid port: " << argv[1] << ". Using 7777.\n";
        } else {
            port = static_cast<uint16_t>(p);
        }
    }

    unsigned thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 2;
    if (argc > 2) {
        int t = std::atoi(argv[2]);
        if (t > 0) thread_count = static_cast<unsigned>(t);
    }

    auto io_ctx = std::make_unique<asio::io_context>();
    auto* io_ctx_ptr = io_ctx.get();

    auto id_gen = std::make_unique<MonotonicIdGenerator>();
    auto clients = std::make_unique<ClientRepository>();
    auto transport = std::make_unique<UdpTransport>(*io_ctx_ptr);
    auto router = std::make_unique<PacketRouter>();

    auto connect = std::make_unique<ConnectHandler>(*id_gen, *clients, *transport);
    auto relay = std::make_unique<RelayHandler>(*clients, *transport);

    auto deps = ServerDependencies{
        .io_ctx = std::move(io_ctx),
        .id_generator = std::move(id_gen),
        .client_repository = std::move(clients),
        .transport = std::move(transport),
        .router = std::move(router),
        .connect_handler = std::move(connect),
        .relay_handler = std::move(relay),
    };

    auto server = std::make_unique<Server>(std::move(deps), port);

    asio::signal_set signals(*io_ctx_ptr, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) { server->Stop(); });

    server->Start();

    std::vector<std::thread> workers;
    workers.reserve(thread_count);
    for (unsigned i = 0; i < thread_count; ++i) workers.emplace_back([io_ctx_ptr] { io_ctx_ptr->run(); });

    for (auto& t : workers) t.join();

    return 0;
}
