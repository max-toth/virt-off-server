#pragma once

#include <string>

namespace v2 {

/// Client-ID generator producing human-readable string identifiers.
///
/// Returns ids such as "player-<timestamp>". Separate from the legacy
/// ::IIdGenerator (numeric); lives in namespace v2 for versioning.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp).
struct IIdGenerator {
    virtual ~IIdGenerator() = default;

    /// Returns the next unique client ID in the form "player-<timestamp>".
    /// When two calls land in the same millisecond, a disambiguating suffix
    /// is appended so the result stays unique. Never repeats within the
    /// process lifetime.
    virtual std::string next() = 0;
};

}  // namespace v2
