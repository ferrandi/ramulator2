# Ramulator 2 for Bambu

A Bambu-oriented fork of [Ramulator 2](https://github.com/CMU-SAFARI/ramulator2),
the cycle-accurate DRAM simulator. This repository carries
[@jesunsahariar's integration](https://github.com/jesunsahariar/Ramulator-no-spdlog)
with a standard build and installation layout for
[Bambu](https://github.com/ferrandi/PandA-bambu) memory co-simulation.

## What changed

- @jesunsahariar's integration replaces the spdlog/fmt dependencies with lightweight logging.
- Out-of-source CMake builds keep downloaded dependencies and generated files in
  the build directory.
- Installation provides the shared library, public headers, executable, and a
  `Ramulator::ramulator` CMake target in one prefix.

This fork uses the older Ramulator 2.0 code line used by the Bambu integration;
it does not track the latest upstream release automatically. Original copyright
notices and the [MIT license](LICENSE) are retained.

## Build and install

Requirements: Git, CMake 3.15 or newer, and a C++20 compiler (for example GCC 12
or Clang 15). CMake downloads yaml-cpp 0.7.0 and argparse 2.9 on the first build,
so network access is required during configuration.

```sh
git clone https://github.com/ferrandi/ramulator2.git
cd ramulator2
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install"
cmake --build build --parallel 4
cmake --install build
./install/bin/ramulator2 --help
```

The prefix contains `bin/ramulator2`, `lib*/libramulator.so`,
`include/ramulator/`, and the installed CMake package. The executable locates its
installed library without an extra `LD_LIBRARY_PATH` setting.

## Use with Bambu

Point the Bambu simulation environment at the installation prefix:

```sh
export RAMULATOR2_DIR="$PWD/install"
```

Then run Bambu with `--enable-ramulator` and the desired bus or AXI interface.
When building through the Bambu submodule, use the prefix created by that build
instead. Bambu pins a specific commit rather than following a moving branch.

## Branches and provenance

- **`main`**: @jesunsahariar's Ramulator version at
  [`d1723a4796e799667391931bed2a3af7fe9b0f26`](https://github.com/jesunsahariar/Ramulator-no-spdlog/commit/d1723a4796e799667391931bed2a3af7fe9b0f26),
  plus Bambu build/install adaptations. @jesunsahariar's initial source import is
  [`1682361bc7e3f14eb1ba530f8d25fa836e5b0bee`](https://github.com/jesunsahariar/Ramulator-no-spdlog/commit/1682361bc7e3f14eb1ba530f8d25fa836e5b0bee).
- **`dev`**: the preserved original baseline at
  [`f01a346510f97f1d07defc6198ceeecaaffa0e7c`](https://github.com/ferrandi/ramulator2/commit/f01a346510f97f1d07defc6198ceeecaaffa0e7c),
  a formatting commit on top of
  [`e442c64b2c0db7afd9d23173925d636ea2895a36`](https://github.com/ferrandi/ramulator2/commit/e442c64b2c0db7afd9d23173925d636ea2895a36).

The upstream source baseline identified by comparison with @jesunsahariar's import is
[`CMU-SAFARI/ramulator2@4a29cac924e4dabc8f84c648b653ec4d3ec53f3f`](https://github.com/CMU-SAFARI/ramulator2/commit/4a29cac924e4dabc8f84c648b653ec4d3ec53f3f)
(7 May 2025). This is also the Ramulator source baseline preserved by `dev`
before its formatting commit. @jesunsahariar restarted the Git history when importing
the sources, so this upstream baseline is reconstructed from source comparison,
not recorded as a shared Git ancestor.

For the original simulator documentation, supported memory standards, and
citation, see the [upstream project](https://github.com/CMU-SAFARI/ramulator2).
