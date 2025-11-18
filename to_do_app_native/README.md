# To-Do App Native (C++17, SDL2 + Dear ImGui)

A native desktop to-do application using SDL2 and Dear ImGui, featuring full CRUD, filters, search, and JSON file persistence.

## Prerequisites

- CMake 3.16+
- C++17 compiler (g++/clang++)
- Internet access for CMake FetchContent (to fetch SDL2, Dear ImGui, and nlohmann/json)
- (Optional) Catch2 single-header at /usr/local/include/catch2/catch.hpp

## Build

```bash
./build.sh
```

- Options:
  - -DBUILD_GUI=ON (default) to build with SDL2 + Dear ImGui UI
  - -DWITH_TESTS=OFF (default)

To customize:
```bash
mkdir -p build && cd build
cmake -S .. -B . -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUI=ON -DWITH_TESTS=OFF
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
