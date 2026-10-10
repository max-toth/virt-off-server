#pragma once

#include "core/IIdGenerator.hpp"

#include <atomic>

class MonotonicIdGenerator final : public IIdGenerator {
public:
    uint64_t next() override;

private:
    std::atomic<uint64_t> next_{1};
};
