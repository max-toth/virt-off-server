#pragma once

#include <string_view>

/// Logging sink for server diagnostics and debugging.
///
/// All calls are expected to be fast (non-blocking).
///
/// Thread-safety: NOT required — called from the io_context thread only.
struct ILogger {
    virtual ~ILogger() = default;

    virtual void info(std::string_view msg) = 0;
    virtual void warn(std::string_view msg) = 0;
    virtual void error(std::string_view msg) = 0;
};
