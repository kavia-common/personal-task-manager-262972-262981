# generated tests fragment - creates a test target and registers with CTest
enable_testing()

# Simple test using Catch2 single header if available on system include path.
# The preview environment install script places Catch2 in /usr/local/include/catch2/catch.hpp
add_executable(tests
  test/test_example.cpp
  test/test_storage.cpp
  src/persistence/Storage.cpp
  src/model/TaskList.cpp
  src/model/Task.cpp
)

target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME sanity COMMAND $<TARGET_FILE:tests>)
add_custom_target(run-tests COMMAND $<TARGET_FILE:tests> DEPENDS tests)
