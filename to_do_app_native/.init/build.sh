#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
# create canonical build.sh if missing (idempotent)
if [ ! -f "$WS/build.sh" ]; then cat > "$WS/build.sh" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
mkdir -p "$WS/build" && cd "$WS/build"
cmake -S "$WS" -B . -DCMAKE_BUILD_TYPE=Debug || { echo 'cmake configure failed' >&2; exit 3; }
cmake --build . -- -j"$(nproc)" || { echo 'build failed' >&2; exit 4; }
SH
chmod +x "$WS/build.sh"; fi
[ -x "$WS/build.sh" ] || { echo 'build.sh missing or not executable' >&2; exit 2; }
# run build
"$WS/build.sh"
# locate tests binary
TEST_BIN=$(find "$WS/build" -type f -executable -name tests -print -quit || true)
[ -n "$TEST_BIN" ] || { echo 'tests binary not found after build' >&2; exit 3; }
echo "OK: tests binary found at $TEST_BIN"
