#pragma once

/// Selects which server API version the process runs.
enum class ApiVersion {
    /// Legacy API: uint64 client ids, endpoint-only registry (global namespace).
    V1,

    /// New API: "player-<timestamp>" ids, full UserInfo records (namespace v2).
    V2,
};
