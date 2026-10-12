#pragma once

#include <string>

namespace v2::game {

/// Descriptor of a single prop sent by the client in the MANIFEST message.
struct PropDescriptor {
    /// Identifier (e.g. hierarchy path), used for matching.
    std::string key;

    /// x, y, z half-extents for spatial validation.
    float half_extents[3]{};

    /// Prop mass in kg (affects throw impulse).
    float mass{};
};

}  // namespace v2::game
