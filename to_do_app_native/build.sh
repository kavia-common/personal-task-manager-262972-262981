#!/usr/bin/env bash
# Ensure robust, non-interactive build. Supports offline/headless builds.
set -euo pipefail

# Workspace root (resolve to script directory to avoid hardcoded path issues)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WS="${SCRIPT_DIR}"

# Options (env override)
: "${BUILD_GUI:=OFF}"        # default OFF to avoid heavy SDL/ImGui fetch in CI
: "${WITH_TESTS:=OFF}"
: "${APP_OFFLINE:=ON}"       # default ON to avoid network in restricted environments
: "${CMAKE_BUILD_TYPE:=Debug}"

mkdir -p "${WS}/build"
cd "${WS}/build"

cmake -S "${WS}" -B . \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
  -DBUILD_GUI="${BUILD_GUI}" \
  -DWITH_TESTS="${WITH_TESTS}" \
  -DAPP_OFFLINE="${APP_OFFLINE}" \
  || { echo 'cmake configure failed' >&2; exit 3; }

cmake --build . -- -j"$(nproc)" || { echo 'build failed' >&2; exit 4; }
