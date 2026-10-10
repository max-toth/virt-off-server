#include "app/CliParser.hpp"
#include "app/ServerFactory.hpp"

#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <memory>
#include <thread>
#include <vector>

int main(int argc, char* argv[]) {
    const CliParser parser;
    const CliOptions opts = parser.parse(argc, argv);

    auto io_ctx = std::make_unique<asio::io_context>();
    auto* io_ctx_ptr = io_ctx.get();

    ServerFactory factory(*io_ctx_ptr, opts.port);
    auto server = factory.create(opts.api);

    asio::signal_set signals(*io_ctx_ptr, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) {
        server->Stop();
        io_ctx_ptr->stop();
    });

    server->Start();

    std::vector<std::thread> workers;
    workers.reserve(opts.threads);
    for (unsigned i = 0; i < opts.threads; ++i) {
        workers.emplace_back([io_ctx_ptr] { io_ctx_ptr->run(); });
    }
    for (auto& t : workers) {
        t.join();
    }

    return 0;
}
