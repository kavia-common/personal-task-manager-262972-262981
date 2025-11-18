# To-Do App Native (C++ GTK4)

This is a minimal native GTK4 C++ scaffold for the personal task manager.

## Prerequisites

- cmake
- pkg-config
- libgtk-4-dev
- A C++17 compiler (g++/clang)
- (Optional) Catch2 single-header at /usr/local/include/catch2/catch.hpp

If building inside the preview environment, dependencies are installed by the common setup.

## Build

Use the provided build.sh:

```bash
./build.sh
```

It will generate `build/` and compile both app and tests (if tests are enabled via cmake/tests.cmake).

## Run

```bash
./build/to_do_app_native
```

## Tests

```bash
( cd build && ctest --output-on-failure )
```

or run the test binary:

```bash
./build/tests
```

