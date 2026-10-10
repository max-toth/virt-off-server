#include "core/MonotonicIdGenerator.hpp"

uint64_t MonotonicIdGenerator::next() {
    return next_.fetch_add(1, std::memory_order_relaxed);
}
