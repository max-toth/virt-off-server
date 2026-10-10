#pragma once

#include <cstdint>

/// Wire-protocol control bytes shared by the server and the Unity client.
///
/// Byte 0 of every datagram identifies the message kind; the remaining bytes
/// are the payload. Centralised here so V1 and V2 code use the same values.
namespace protocol {

/// 0xFF — client → server: request to connect.
inline constexpr uint8_t MSG_CONNECT = 0xFF;

/// 0xFE — server → client: assigned client ID.
/// V1 payload: 8-byte native-endian uint64 id.
/// V2 payload: UTF-8 string id (see ConnectHandlerV2).
inline constexpr uint8_t MSG_ASSIGN_ID = 0xFE;

/// 0x01 — client → server → other clients: voice packet (Opus), relayed as-is.
inline constexpr uint8_t MSG_VOICE = 0x01;

/// 0x02 — client → server → other clients: position-sync packet, relayed as-is.
inline constexpr uint8_t MSG_POSITION = 0x02;

}  // namespace protocol
