# Ramulator2 without spdlog

This fork replaces the spdlog/fmt logging dependencies with simple logging for
integration with the Bambu HLS MDPI driver. The memory simulator implementation
is unchanged by the build and installation cleanup.

## Build

A C++20 compiler, CMake 3.15 or newer, and Git are required. CMake downloads the
pinned yaml-cpp and argparse dependencies into the build directory.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/ramulator2 --help
```

The executable and shared library are created in `build/` (or the selected
configuration directory with a multi-configuration generator). Out-of-source
builds do not create artifacts or dependency checkouts in the source tree.

## Install

```sh
cmake --install build --prefix "$HOME/.local"
```

The installation uses the standard CMake `GNUInstallDirs` layout:

- `bin/ramulator2`: simulator executable;
- `lib/` (or the configured `CMAKE_INSTALL_LIBDIR`): versioned shared library;
- `include/ramulator/`: public headers;
- `lib/cmake/Ramulator/`: CMake package and exported target;
- `share/licenses/ramulator/LICENSE`: upstream MIT license.

The bundled yaml-cpp dependency installs its headers and library alongside
Ramulator and is exported as `Ramulator::yaml-cpp`. The CLI has a relative runtime library path and can run
from the installation without setting `LD_LIBRARY_PATH`.

CMake consumers can use the installed package:

```cmake
find_package(Ramulator 2 CONFIG REQUIRED)
target_link_libraries(my_simulator PRIVATE Ramulator::ramulator)
```

Pass `-DCMAKE_PREFIX_PATH=/path/to/prefix` when the prefix is outside CMake's
normal search paths. Packaging supports `DESTDIR` and the standard install
directory overrides such as `-DCMAKE_INSTALL_LIBDIR=lib64`.

For Bambu MDPI integration, set `RAMULATOR2_DIR` to the installation prefix.
The Bambu driver uses the installed headers and shared library; it does not
need access to the Ramulator source or build directory.
