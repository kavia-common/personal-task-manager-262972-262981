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

## Features

- Import/Export JSON (File menu). If OS file pickers are unavailable, a path dialog is provided.
- Confirm Delete dialog and Clear Completed with confirmation.
- Keyboard shortcuts:
  - Enter in Add Task title adds the task.
  - Ctrl+S saves immediately and shows a "Saved" indicator.
  - Ctrl+F focuses the search box.
- Auto-backup and recovery:
  - On every save, creates rotating backups: tasks.json.bak1, .bak2, .bak3.
  - On load failure/malformed JSON, attempts automatic recovery from latest backup.
- Settings persistence:
  - Window geometry (position/size) and theme toggle (light/dark) are stored in settings.json.
  - Restored on next start.
- Ocean Professional theme polish: blue (#2563EB) primary and amber (#F59E0B) secondary accents, subtle rounding/separators, hover states.

## Persistence

Data files are stored under the app data directory:
- Linux: ~/.local/share/to_do_app_native/
- macOS: ~/Library/Application Support/to_do_app_native/
- Windows: %APPDATA%\to_do_app_native\

Files:
- tasks.json: current tasks
- tasks.json.bak1..bak3: rotating backups (bak1 is the most recent)
- settings.json: window geometry and theme

## Import/Export

- Use File -> Import... / Export... to load/save a JSON file.
- The dialog accepts a path; supply a full path like /home/user/tasks_backup.json or C:\Users\you\Documents\tasks_backup.json.
- Export validates and writes with pretty-printed JSON.

## Tests

If enabled:
```bash
( cd build && ctest --output-on-failure )
```

Headless tests include storage JSON round-trip and backup rotation logic (non-UI).
