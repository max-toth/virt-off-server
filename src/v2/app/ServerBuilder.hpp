#pragma once

#include "app/IServerBuilder.hpp"

#include <asio/io_context.hpp>
#include <cstdint>
#include <memory>

namespace v2 {

/// Builds the V2 (string-id, full user records) server stack.
///
/// Owns no io_context: the caller keeps the event loop alive; this builder
/// only wires transports, handlers and the server.
class ServerBuilder final : public IServerBuilder {
public:
    ServerBuilder(asio::io_context& io_ctx, uint16_t port);

    std::unique_ptr<IServer> build() override;

private:
    /// Event loop used by the transport. Not owned.
    asio::io_context& io_ctx_;

    /// UDP port the server listens on.
    uint16_t port_;
};

}  // namespace v2
