# To-Do App Native (C++17, SDL2 + Dear ImGui)

A native desktop to-do application using SDL2 and Dear ImGui, featuring full CRUD, filters, search, and JSON file persistence.

## Prerequisites

- CMake 3.16+
- C++17 compiler (g++/clang++)
- Internet access for CMake FetchContent (to fetch SDL2, Dear ImGui, and nlohmann/json) unless APP_OFFLINE=ON
- (Optional) Catch2 single-header at /usr/local/include/catch2/catch.hpp

## Build

```bash
./build.sh
```

- Options:
  - Environment variables:
    - BUILD_GUI=ON|OFF (default OFF in CI script to avoid heavy fetches)
    - WITH_TESTS=ON|OFF (default OFF)
    - APP_OFFLINE=ON|OFF (default ON to avoid network; set OFF to allow FetchContent)
    - CMAKE_BUILD_TYPE=Debug|Release (default Debug)

Examples:
- Headless, offline (fast, default):
```bash
BUILD_GUI=OFF APP_OFFLINE=ON ./build.sh
```

- GUI build with network fetch allowed:
```bash
BUILD_GUI=ON APP_OFFLINE=OFF ./build.sh
```

To customize using raw CMake:
```bash
mkdir -p build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=ON -DWITH_TESTS=OFF -DAPP_OFFLINE=OFF
cmake --build . -- -j"$(nproc)"
```

## Run

```bash
./build/to_do_app_native
```

## Persistence

Tasks are stored as JSON at:
- Linux: ~/.local/share/to_do_app_native/tasks.json
- macOS: ~/Library/Application Support/to_do_app_native/tasks.json
- Windows: %APPDATA%\to_do_app_native\tasks.json

Malformed files are backed up to tasks.json.bak on load.

## Tests

If enabled:
```bash
( cd build && ctest --output-on-failure )
```
