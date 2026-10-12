#pragma once

#include <cstdint>
#include <string>

namespace v2::game {

/// Server-side canonical record for a single prop.
///
/// Owner is authoritative — only the holder may send PROP_STATE.
/// `held == false` means the prop is resting at its last-released position.
struct PropRecord {
    /// Canonical id assigned by server in MANIFEST_ACK.
    uint32_t prop_id{};

    /// Matches PropDescriptor::key from manifest.
    std::string key;

    /// Empty string means free (no owner).
    std::string owner_id;

    /// True while the owner actively holds the prop.
    bool held{};
};

}  // namespace v2::game
