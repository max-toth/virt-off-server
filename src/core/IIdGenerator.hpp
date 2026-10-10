#pragma once

#include <cstdint>

/// Strictly monotonic client ID generator.
///
/// Thread-safety: NOT required — called from the io_context thread only.
/// Future: wrap in strand or switch to atomic generator.
struct IIdGenerator {
    virtual ~IIdGenerator() = default;

    /// Returns the next unique identifier. Never returns the same value twice
    /// within the same process lifetime.
    virtual uint64_t next() = 0;
};
