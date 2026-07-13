# Contributing

Everyday commands are CMake workflow presets, one verb per task:

| Command                              | What it does                                           |
|--------------------------------------|--------------------------------------------------------|
| `cmake --workflow --preset build`    | Compile everything for the host (Debug + ASan/UBSan)   |
| `cmake --workflow --preset test`     | Build and run the unit tests                           |
| `cmake --workflow --preset lint`     | Compile with clang-tidy (config: `.clang-tidy`)        |
| `cmake --workflow --preset fmt`      | clang-format (+ cmake-format if installed) in place    |
| `cmake --workflow --preset coverage` | Run tests instrumented, print + render coverage report |
| `cmake --workflow --preset fuzz`     | Build the libFuzzer targets                            |

Each preset configures and builds in its own directory under `build/<preset>`,
so switching between them never invalidates another preset's cache.

## Prerequisites

- **Always**: CMake ≥ 3.28 and Ninja. The default host compiler must speak
  C++23 (`std::expected` needs GCC >= 12 or Clang >= 19 with libstdc++).
- **`lint`**: `clang-tidy` on the `PATH`.
- **`fmt`**: `clang-format` (config: `.clang-format`); `cmake-format` is
  optional and picked up when present.
- **`coverage`**: with Clang, `llvm-cov` + `llvm-profdata`; with GCC,
  `lcov`. The report lands in `build/coverage/ccov/` and a summary is
  printed to the terminal.
- **`fuzz`**: LLVM Clang (Apple Clang lacks libFuzzer). If your default
  compiler is not LLVM Clang, override it:
  `cmake --preset fuzz -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++`,
  or keep a personal preset in `CMakeUserPresets.json` (gitignored) that
  inherits `fuzz` and pins the compiler.

## Build options

Every knob is a `CHELSSY_*` option declared at the top of the root
`CMakeLists.txt` and handled by the same-named module in `cmake/`
(`Sanitizers.cmake`, `Coverage.cmake`, …). Tooling helpers come from
[StableCoder cmake-scripts](https://github.com/StableCoder/cmake-scripts),
fetched pinned at configure time; the modules in `cmake/` are thin wrappers
around it.

## Architecture

Design decisions are recorded as ADRs in [`docs/adr/`](docs/adr/README.md) -
start with 0005 (ports and adapters) and 0008 (project layout).
