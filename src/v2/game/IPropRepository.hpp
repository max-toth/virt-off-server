#pragma once

#include "v2/game/PropDescriptor.hpp"
#include "v2/game/PropRecord.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace v2::game {

/// Canonical prop registry and ownership arbiter.
///
/// One record per prop. On grant the server verifies the prop is free;
/// on release it clears the owner. All changes are authoritative.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
struct IPropRepository {
    virtual ~IPropRepository() = default;

    /// Registers props from a client manifest. Existing props with the
    /// same key are NOT replaced (first manifest wins). Assigns prop_ids.
    virtual void register_manifest(const std::vector<PropDescriptor>& descriptors) = 0;

    /// Atomically grants `prop_id` to `holder_id` if the prop is free.
    /// Returns true on success, false if already held.
    virtual bool try_grant(uint32_t prop_id, const std::string& holder_id) = 0;

    /// Releases `prop_id` — clears owner, sets held=false.
    /// Returns the record before release, or nullopt if unknown.
    virtual std::optional<PropRecord> release(uint32_t prop_id) = 0;

    /// Releases every prop owned by `owner_id` (used on disconnect).
    virtual void release_all_of(const std::string& owner_id) = 0;

    /// Returns a copy of the record for `prop_id`, or nullopt.
    virtual std::optional<PropRecord> get(uint32_t prop_id) const = 0;

    /// Invokes `fn` for every registered prop.
    virtual void for_each(const std::function<void(const PropRecord&)>& fn) const = 0;

    /// Returns the total number of registered props.
    virtual size_t size() const = 0;
};

}  // namespace v2::game
