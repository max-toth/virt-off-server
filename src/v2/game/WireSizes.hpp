#pragma once

#include <cstddef>
#include <cstdint>

namespace v2::game {

// -- Wire-protocol primitive field sizes (bytes) --

/// uint32 LE — prop id, count, etc.
inline constexpr size_t kU32Bytes    = 4;

/// uint16 LE — count fields.
inline constexpr size_t kU16Bytes    = 2;

/// 3 × float — position, velocity, angular velocity, half-extents.
inline constexpr size_t kVec3Bytes   = 12;

/// 4 × float — quaternion rotation.
inline constexpr size_t kQuatBytes   = 16;

/// Attack kind byte.
inline constexpr size_t kKindBytes   = 1;

/// Hit-object index byte.
inline constexpr size_t kHitObjBytes = 1;

/// Length prefix of a length-prefixed string (uint8).
inline constexpr size_t kStrLenBytes = 1;

/// Generic flags byte.
inline constexpr size_t kFlagsBytes  = 1;

// -- Flag bit values --

/// Bit 0 of PROP_RELEASE_EVENT flags: prop has settled / is FREE.
inline constexpr uint8_t kPropFlagSettled = 0x01;

// -- Message payload layout helpers --

/// Offsets within a PROP_STATE payload (after type byte):
/// [id:str|propId:4|pos:12|rot:16|vel:12|flags:1]
/// String layout: [len:1][utf8…]. Offsets below account for the string header only;
/// the caller must add str_len after reading the id.
inline constexpr size_t kPropStatePropIdOff = kStrLenBytes;  // past id length byte

/// Offsets within a POSITION payload (after type byte):
/// [id:str|pos:12|rot:16|vel:12]
inline constexpr size_t kPosPosOff    = kStrLenBytes;         // past id length byte

/// Offsets within a PROP_GRANT payload:
/// [propId:4][holderId:str][flags:1]
inline constexpr size_t kGrantHolderIdOff = kU32Bytes;        // past propId

/// Offsets within a PROP_RELEASE_EVENT payload:
/// [propId:4][ownerId:str][vel:12][angVel:12][flags:1]
/// Owner id is empty (len=0) when FREE.
inline constexpr size_t kReleaseEventOwnerIdOff = kU32Bytes;  // past propId

}  // namespace v2::game
