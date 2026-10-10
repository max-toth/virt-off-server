#pragma once

#include "app/ApiVersion.hpp"

#include <cstdint>

/// Parsed command-line options for the server executable.
///
/// Plain data: resolved once from argv and read-only afterwards.
struct CliOptions {
    /// UDP port used when --port is not supplied.
    static constexpr uint16_t kDefaultPort = 7777;

    /// UDP port the server should listen on.
    uint16_t port = kDefaultPort;

    /// Number of io_context worker threads.
    unsigned threads = 0;

    /// API version the server should serve.
    ApiVersion api = ApiVersion::V1;
};
