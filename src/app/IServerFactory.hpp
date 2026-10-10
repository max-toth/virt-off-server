#pragma once

#include "app/ApiVersion.hpp"
#include "app/Server.hpp"

#include <memory>

/// Version-agnostic entry point for creating a server.
///
/// Hides from callers which API version is running behind the common
/// ::IServer interface.
struct IServerFactory {
    virtual ~IServerFactory() = default;

    /// Builds and returns the server for `version`. Caller owns the result.
    virtual std::unique_ptr<IServer> create(ApiVersion version) = 0;
};
