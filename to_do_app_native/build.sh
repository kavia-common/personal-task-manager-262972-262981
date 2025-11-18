#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
mkdir -p "$WS/build" && cd "$WS/build"
cmake -S "$WS" -B . -DCMAKE_BUILD_TYPE=Debug || { echo 'cmake configure failed' >&2; exit 3; }
cmake --build . -- -j"$(nproc)" || { echo 'build failed' >&2; exit 4; }
