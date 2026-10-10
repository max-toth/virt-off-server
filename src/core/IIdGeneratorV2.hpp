#pragma once

#include <string>

/// V2 client-ID generator producing human-readable string identifiers.
///
/// Unlike the V1 `IIdGenerator` (numeric), V2 returns a string such as
/// "player-<...>". Kept as a separate interface for backward compatibility:
/// the V1 generator and all V1 consumers remain untouched.
///
/// Thread-safety: REQUIRED — may be called concurrently from multiple
/// io_context worker threads (see main.cpp).
struct IIdGeneratorV2 {
    virtual ~IIdGeneratorV2() = default;

    /// Returns the next unique client ID in the form "player-<timestamp>".
    /// When two calls land in the same millisecond, an additional
    /// disambiguating suffix is appended so the result stays unique.
    /// Never returns the same value twice within the same process lifetime.
    virtual std::string next() = 0;
};
