#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
mkdir -p "$WS/test" "$WS/cmake" && cd "$WS"
cat > "$WS/test/test_example.cpp" <<'CPP'
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
TEST_CASE("sanity") { REQUIRE(1==1); }
CPP
cat > "$WS/cmake/tests.cmake" <<'CM'
# generated tests fragment - creates a test target and registers with CTest
enable_testing()
add_executable(tests test/test_example.cpp)
# Ensure /usr/local/include is searched if Catch2 is placed there (compiler default on Ubuntu typically includes it)
# Link GTK if available to ensure correct linking flags
if(TARGET PkgConfig::GTK4)
  target_link_libraries(tests PRIVATE PkgConfig::GTK4)
elseif(GTK4_FOUND)
  target_include_directories(tests PRIVATE ${GTK4_INCLUDE_DIRS})
  target_link_libraries(tests PRIVATE ${GTK4_LIBRARIES})
endif()
add_test(NAME sanity COMMAND $<TARGET_FILE:tests>)
add_custom_target(run-tests COMMAND $<TARGET_FILE:tests> DEPENDS tests)
CM
