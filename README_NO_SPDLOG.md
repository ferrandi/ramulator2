# Ramulator2 - spdlog-free Version

## Overview

This is a modified copy of Ramulator2 with all spdlog and fmt dependencies removed to resolve compilation conflicts with Bambu HLS MDPI integration.

## Original Source
- **Original location:** `/workspaces/soda-benchmarks-main/external/ramulator2/ramulator2-main`
- **Creation date:** December 10, 2024
- **Purpose:** Eliminate spdlog/fmt compilation conflicts with MDPI

## Modifications Made

### 1. Logging Replacement
- All `#include <spdlog/spdlog.h>` replaced with simple logging
- All `spdlog::` calls replaced with basic stdout/stderr output
- All logging functionality simplified to printf-style

### 2. String Formatting Replacement
- All `fmt::format()` calls replaced with:
  - `std::to_string()` for simple cases
  - `sprintf()` / `snprintf()` for complex formatting
  - String concatenation for simple cases

### 3. Build System Changes
- Removed spdlog dependency from CMakeLists.txt
- Removed fmt dependency requirements
- Updated include paths to exclude bundled libraries

## Files Modified
(This will be populated as modifications are made)

## Build Instructions
```bash
cd /workspaces/soda-benchmarks-main/external/ramulator2/ramulator2-no-spdlog
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
make -j$(nproc)
```

## Integration with MDPI
This version can be used with MDPI by updating:
- `RAMULATOR2_DIR` in Makefile.mk to point to this directory
- Include paths to use this spdlog-free version
- Link against the clean libramulator.so from this build

## Status
🚧 **IN PROGRESS** - Modifications underway

---
**Note:** This is a temporary fork for integration purposes. Production use should consider proper logging integration.