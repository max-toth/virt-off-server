#pragma once

#include "app/CliOptions.hpp"

/// Parses the process command line into CliOptions.
///
/// Implementations write diagnostics only to stderr and never throw; unknown
/// or malformed options are ignored, leaving the corresponding default in
/// place. No shared mutable state — safe to call from the composition root.
struct ICliParser {
    virtual ~ICliParser() = default;

    /// Parses `argc`/`argv` into resolved options.
    ///
    /// @param argc argument count as passed to main (>= 0)
    /// @param argv argument vector as passed to main; argv[0] is the program name
    /// @return fully populated options with defaults for absent/invalid values
    virtual CliOptions parse(int argc, char* argv[]) const = 0;
};
