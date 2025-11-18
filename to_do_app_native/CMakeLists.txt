cmake_minimum_required(VERSION 3.16)
project(to_do_app_native LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find GTK4 via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

# Main application target
add_executable(to_do_app_native
    src/main.cpp
)

# Link against gtk4 imported target if available, otherwise fallback to vars
if (TARGET PkgConfig::GTK4)
    target_link_libraries(to_do_app_native PRIVATE PkgConfig::GTK4)
else()
    if (GTK4_FOUND)
        target_include_directories(to_do_app_native PRIVATE ${GTK4_INCLUDE_DIRS})
        target_link_libraries(to_do_app_native PRIVATE ${GTK4_LIBRARIES})
        if (GTK4_CFLAGS_OTHER)
            target_compile_options(to_do_app_native PRIVATE ${GTK4_CFLAGS_OTHER})
        endif()
    else()
        message(FATAL_ERROR "GTK4 pkg-config not found; please install libgtk-4-dev")
    endif()
endif()

# Tests integration (CTest)
include(CTest)
if (EXISTS "${CMAKE_SOURCE_DIR}/cmake/tests.cmake")
    include(${CMAKE_SOURCE_DIR}/cmake/tests.cmake)
endif()
