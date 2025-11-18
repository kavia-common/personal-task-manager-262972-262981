# generated tests fragment - creates a test target and registers with CTest
enable_testing()

# Simple test using Catch2 single header if available on system include path.
# The preview environment install script places Catch2 in /usr/local/include/catch2/catch.hpp
add_executable(tests test/test_example.cpp)

# Link GTK if available (for consistency with compile/link flags)
if (TARGET PkgConfig::GTK4)
  target_link_libraries(tests PRIVATE PkgConfig::GTK4)
elseif (GTK4_FOUND)
  target_include_directories(tests PRIVATE ${GTK4_INCLUDE_DIRS})
  target_link_libraries(tests PRIVATE ${GTK4_LIBRARIES})
endif()

add_test(NAME sanity COMMAND $<TARGET_FILE:tests>)
add_custom_target(run-tests COMMAND $<TARGET_FILE:tests> DEPENDS tests)
