#pragma once

#include "core/IIdGeneratorV2.hpp"

#include <cstdint>
#include <mutex>
#include <string>

/// Thread-safe V2 client-ID generator producing "player-<unix_millis>".
///
/// Ids are derived from the wall-clock timestamp. Two consecutive calls that
/// land in the same millisecond get an extra "-<n>" suffix, so the result is
/// always unique within the process lifetime.
class TimestampIdGeneratorV2 final : public IIdGeneratorV2 {
public:
    std::string next() override;

private:
    /// Returns the current wall-clock time in milliseconds since Unix epoch.
    static uint64_t current_millis();

    /// Guards last_millis_ and same_millis_counter_.
    std::mutex mutex_;

    /// Timestamp (ms) of the most recently generated id.
    uint64_t last_millis_ = 0;

    /// Number of ids already generated with last_millis_.
    uint64_t same_millis_counter_ = 0;
};
