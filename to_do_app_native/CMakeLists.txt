cmake_minimum_required(VERSION 3.16)
project(to_do_app_native LANGUAGES CXX)

# Options
option(BUILD_GUI "Build with GUI" ON)
option(WITH_TESTS "Build tests" OFF)
option(APP_OFFLINE "Do not fetch dependencies from network; use system/vendor copies only" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependency discovery helpers
include(FetchContent)

# nlohmann/json single-header
# Prefer system-installed header when offline or available.
set(JSON_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann")
set(JSON_VENDOR_HEADER "${JSON_VENDOR_DIR}/json.hpp")

set(HAVE_NLOHMANN_JSON FALSE)
if (EXISTS "/usr/include/nlohmann/json.hpp")
  # System style directory contains nlohmann/json.hpp, but our code includes "json.hpp".
  # Add directory so that "json.hpp" resolves to /usr/include/nlohmann/json.hpp via -I/usr/include/nlohmann
  target_include_directories  # placeholder to keep syntax aligned
endif()

# We will defer include dirs until target exists; record include hints:
set(_json_include_hints "")
if (EXISTS "/usr/include/nlohmann/json.hpp")
  list(APPEND _json_include_hints "/usr/include/nlohmann")
  set(HAVE_NLOHMANN_JSON TRUE)
elseif (EXISTS "${JSON_VENDOR_HEADER}")
  list(APPEND _json_include_hints "${JSON_VENDOR_DIR}")
  set(HAVE_NLOHMANN_JSON TRUE)
endif()

if (NOT APP_OFFLINE AND NOT HAVE_NLOHMANN_JSON)
  # Fallback to fetching single header
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
    DOWNLOAD_NO_EXTRACT TRUE
  )
  FetchContent_MakeAvailable(nlohmann_json)
  set(HAVE_NLOHMANN_JSON TRUE)
endif()

# SDL2 + ImGui only when GUI enabled
set(SDL2_LIB "")
if (BUILD_GUI)
  # Try system SDL2 first
  find_package(SDL2 QUIET)
  if (SDL2_FOUND)
    set(SDL2_LIB SDL2::SDL2)
  elseif(NOT APP_OFFLINE)
    # Fetch SDL2 if network allowed
    FetchContent_Declare(
      SDL2
      URL https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.tar.gz
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_LIB SDL2::SDL2 SDL2::SDL2main)
  else()
    message(FATAL_ERROR "APP_OFFLINE=ON and SDL2 not found on system. Install libsdl2-dev or disable GUI via -DBUILD_GUI=OFF.")
  endif()

  # Dear ImGui
  if (NOT APP_OFFLINE)
    FetchContent_Declare(
      imgui
      GIT_REPOSITORY https://github.com/ocornut/imgui.git
      GIT_TAG v1.91.0
    )
    FetchContent_MakeAvailable(imgui)
  else()
    message(STATUS "APP_OFFLINE=ON: ImGui not fetched. GUI build requires a local vendor checkout. Disable GUI with -DBUILD_GUI=OFF to proceed.")
  endif()
endif()

# Sources
set(APP_SOURCES
  src/main.cpp
  src/app/Application.cpp
  src/ui/Ui.cpp
  src/model/Task.cpp
  src/model/TaskList.cpp
  src/persistence/Storage.cpp
)

set(APP_HEADERS
  src/app/Application.h
  src/ui/Ui.h
  src/model/Task.h
  src/model/TaskList.h
  src/persistence/Storage.h
  src/persistence/Json.h
)

add_executable(to_do_app_native ${APP_SOURCES} ${APP_HEADERS})

# Include dirs
target_include_directories(to_do_app_native PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Provide nlohmann/json header include path using discovered/source dirs
# Prefer system/vendor/include hints, otherwise use fetched header dir.
if (HAVE_NLOHMANN_JSON)
  target_include_directories(to_do_app_native PRIVATE ${_json_include_hints})
elseif (DEFINED nlohmann_json_SOURCE_DIR)
  # The file is downloaded into ${nlohmann_json_SOURCE_DIR}/json.hpp
  target_include_directories(to_do_app_native PRIVATE ${nlohmann_json_SOURCE_DIR})
else()
  message(FATAL_ERROR "nlohmann/json header not available. Provide system package (e.g., nlohmann-json-dev), vendor header at third_party/nlohmann/json.hpp, or allow network fetch by APP_OFFLINE=OFF.")
endif()

# GUI vs headless compile definitions
if (BUILD_GUI)
  target_compile_definitions(to_do_app_native PRIVATE APP_WITH_GUI=1)
else()
  target_compile_definitions(to_do_app_native PRIVATE APP_WITH_GUI=0)
endif()

# Link libraries for GUI
if (BUILD_GUI)
  # ImGui sources (fetched)
  if (DEFINED imgui_SOURCE_DIR)
    add_library(dear_imgui
      ${imgui_SOURCE_DIR}/imgui.cpp
      ${imgui_SOURCE_DIR}/imgui_draw.cpp
      ${imgui_SOURCE_DIR}/imgui_tables.cpp
      ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
    )
    target_include_directories(dear_imgui PUBLIC
      ${imgui_SOURCE_DIR}
      ${imgui_SOURCE_DIR}/backends
    )
    # Link SDL2 to dear_imgui for backend if available
    if (TARGET SDL2::SDL2)
      target_link_libraries(dear_imgui PUBLIC SDL2::SDL2)
    endif()
  else()
    message(FATAL_ERROR "ImGui sources not available. In APP_OFFLINE, vendor ImGui locally or build with -DBUILD_GUI=OFF.")
  endif()

  # OpenGL (for ImGui OpenGL2 backend)
  find_package(OpenGL REQUIRED)

  target_link_libraries(to_do_app_native PRIVATE
    ${SDL2_LIB}
    dear_imgui
    OpenGL::GL
  )
endif()

# Platform-specifics for Windows console
if (WIN32 AND BUILD_GUI)
  # Hide console window for GUI app
  set_target_properties(to_do_app_native PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# Tests integration (CTest)
include(CTest)
if (WITH_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/cmake/tests.cmake")
    include(${CMAKE_SOURCE_DIR}/cmake/tests.cmake)
endif()
