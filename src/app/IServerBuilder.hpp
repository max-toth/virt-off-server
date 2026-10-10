#pragma once

#include "app/Server.hpp"

#include <memory>

/// Strategy that builds the fully-wired server for one specific API version.
///
/// One implementation per version (V1ServerBuilder, V2ServerBuilder). The
/// version-agnostic ServerFactory selects the right strategy at runtime.
struct IServerBuilder {
    virtual ~IServerBuilder() = default;

    /// Builds and returns the server. Caller owns the result.
    virtual std::unique_ptr<IServer> build() = 0;
};
