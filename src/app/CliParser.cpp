#include "app/CliParser.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <thread>

namespace {

/// Option flag selecting the UDP port.
constexpr std::string_view kPortFlag = "--port";

/// Option flag selecting the worker-thread count.
constexpr std::string_view kThreadsFlag = "--threads";

/// Option flag selecting the API version.
constexpr std::string_view kApiFlag = "--api";

/// Value of --api that selects the legacy API.
constexpr std::string_view kApiV1 = "v1";

/// Value of --api that selects the V2 API.
constexpr std::string_view kApiV2 = "v2";

/// Largest valid UDP port number.
constexpr int kMaxPortValue = 65535;

/// Worker-thread count used when hardware concurrency is unknown.
constexpr unsigned kFallbackThreadCount = 2;

/// Resolves the default worker-thread count from the hardware.
unsigned default_thread_count() {
    const unsigned hw = std::thread::hardware_concurrency();
    if (hw == 0) {
        return kFallbackThreadCount;
    }
    return hw;
}

}  // namespace

CliOptions CliParser::parse(int argc, char* argv[]) const {
    CliOptions opts;
    opts.threads = default_thread_count();

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        const bool has_value = (i + 1 < argc);

        if (arg == kPortFlag && has_value) {
            const int value = std::atoi(argv[++i]);
            if (value > 0 && value <= kMaxPortValue) {
                opts.port = static_cast<uint16_t>(value);
            }
        } else if (arg == kThreadsFlag && has_value) {
            const int value = std::atoi(argv[++i]);
            if (value > 0) {
                opts.threads = static_cast<unsigned>(value);
            }
        } else if (arg == kApiFlag && has_value) {
            const std::string_view value = argv[++i];
            if (value == kApiV2) {
                opts.api = ApiVersion::V2;
            } else if (value == kApiV1) {
                opts.api = ApiVersion::V1;
            }
        } else {
            std::cerr << "Ignoring unknown argument: " << arg << '\n';
        }
    }

    return opts;
}
