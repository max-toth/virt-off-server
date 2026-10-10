#pragma once

#include "v2/core/IIdGenerator.hpp"

#include <cstdint>
#include <mutex>
#include <string>

namespace v2 {

/// Thread-safe id generator: "player-<unix_millis>".
///
/// Two calls in the same millisecond get an extra "-<n>" suffix so ids stay
/// unique within the process lifetime.
class TimestampIdGenerator final : public IIdGenerator {
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

}  // namespace v2
