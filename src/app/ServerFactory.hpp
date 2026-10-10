#pragma once

#include "app/ApiVersion.hpp"
#include "app/IServerBuilder.hpp"
#include "app/IServerFactory.hpp"

#include <asio/io_context.hpp>
#include <cstdint>
#include <memory>

/// Version-agnostic server factory.
///
/// Holds one IServerBuilder per supported API version and dispatches
/// create() to the matching strategy. The io_context is borrowed from the
/// caller and is not owned by this factory or by the produced servers.
class ServerFactory final : public IServerFactory {
public:
    ServerFactory(asio::io_context& io_ctx, uint16_t port);

    std::unique_ptr<IServer> create(ApiVersion version) override;

private:
    /// Strategy for the legacy V1 API.
    std::unique_ptr<IServerBuilder> legacy_;

    /// Strategy for the V2 API.
    std::unique_ptr<IServerBuilder> next_;
};
