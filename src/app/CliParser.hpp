#pragma once

#include "app/ICliParser.hpp"

/// Default command-line parser.
///
/// Recognizes `--port N`, `--threads N` and `--api v1|v2`; anything else is
/// reported on stderr and skipped. Thread count defaults to the hardware
/// concurrency when --threads is absent.
class CliParser final : public ICliParser {
public:
    CliOptions parse(int argc, char* argv[]) const override;
};
