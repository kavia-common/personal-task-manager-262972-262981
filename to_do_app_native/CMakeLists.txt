cmake_minimum_required(VERSION 3.16)
project(to_do_app_native LANGUAGES CXX)

# Options
option(BUILD_GUI "Build with GUI" ON)
option(WITH_TESTS "Build tests" OFF)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# FetchContent for dependencies
include(FetchContent)

# nlohmann/json single-header
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
  DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# SDL2
if (BUILD_GUI)
  # Try to find system SDL2 first for faster builds; fall back to FetchContent if not found
  find_package(SDL2 QUIET)
  if (NOT SDL2_FOUND)
    FetchContent_Declare(
      SDL2
      URL https://github.com/libsdl-org/SDL/releases/download/release-2.28.5/SDL2-2.28.5.tar.gz
    )
    FetchContent_MakeAvailable(SDL2)
    set(SDL2_LIB SDL2::SDL2 SDL2::SDL2main)
  else()
    set(SDL2_LIB SDL2::SDL2)
  endif()

  # Dear ImGui + SDL2/GL2 backend (embed minimal sources)
  FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
  )
  FetchContent_MakeAvailable(imgui)
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

# Provide nlohmann/json header include path using interface include dir
# We'll copy json.hpp into build dir via FetchContent; add include path:
if (nlohmann_json_POPULATED)
  # The file is downloaded into ${nlohmann_json_SOURCE_DIR}/json.hpp
  target_include_directories(to_do_app_native PRIVATE ${nlohmann_json_SOURCE_DIR})
endif()

# GUI vs headless compile definitions
if (BUILD_GUI)
  target_compile_definitions(to_do_app_native PRIVATE APP_WITH_GUI=1)
else()
  target_compile_definitions(to_do_app_native PRIVATE APP_WITH_GUI=0)
endif()

# Link libraries
if (BUILD_GUI)
  # ImGui sources
  # Build a static library for Dear ImGui
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
