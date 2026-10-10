#include "core/TimestampIdGeneratorV2.hpp"

#include <chrono>

namespace {
/// Prefix prepended to every generated id.
constexpr const char* kPrefix = "player-";

/// Separator between the timestamp and the same-millisecond disambiguator.
constexpr const char* kSeparator = "-";
}  // namespace

uint64_t TimestampIdGeneratorV2::current_millis() {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

std::string TimestampIdGeneratorV2::next() {
    std::lock_guard<std::mutex> lock(mutex_);

    const uint64_t millis = current_millis();
    if (millis == last_millis_) {
        ++same_millis_counter_;
    } else {
        last_millis_ = millis;
        same_millis_counter_ = 0;
    }

    std::string id = std::string(kPrefix) + std::to_string(millis);
    if (same_millis_counter_ > 0) {
        id += std::string(kSeparator) + std::to_string(same_millis_counter_);
    }
    return id;
}
