#pragma once

#include "v2/game/IPropRepository.hpp"

#include <string>
#include <vector>

namespace v2::game {

/// In-memory prop registry.
///
/// Props are stored in insertion order. prop_ids are 1-based indices into the
/// vector (0 is invalid / none). First manifest wins — duplicates by key are
/// silently ignored.
///
/// Thread-safety: NOT required — accessed only on the transport strand.
class PropRepository final : public IPropRepository {
public:
    void register_manifest(const std::vector<PropDescriptor>& descriptors) override;

    bool try_grant(uint32_t prop_id, const std::string& holder_id) override;

    std::optional<PropRecord> release(uint32_t prop_id) override;

    void release_all_of(const std::string& owner_id) override;

    std::optional<PropRecord> get(uint32_t prop_id) const override;

    void for_each(const std::function<void(const PropRecord&)>& fn) const override;

    size_t size() const override;

private:
    /// Ordered prop records; index + 1 = prop_id.
    std::vector<PropRecord> records_;
};

}  // namespace v2::game
