#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
command -v sudo >/dev/null || { echo 'sudo required' >&2; exit 2; }
PKGS=(cmake pkg-config libx11-dev libgtk-4-dev libsqlite3-dev)
[ "${DEBUG:-}" = "true" ] && PKGS+=(gdb)
TO_INSTALL=()
for p in "${PKGS[@]}"; do
  dpkg-query -W -f='${Status}' "$p" 2>/dev/null | grep -q "install ok installed" || TO_INSTALL+=("$p")
done
if [ ${#TO_INSTALL[@]} -ne 0 ]; then
  tries=0
  until sudo apt-get update -q && sudo DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends -qq "${TO_INSTALL[@]}"; do
    tries=$((tries+1))
    [ "$tries" -ge 3 ] && { echo 'apt install failed' >&2; exit 3; }
    sleep 2
  done
fi
# verify cmake and ctest
command -v cmake >/dev/null || { echo 'cmake not available' >&2; exit 4; }
command -v ctest >/dev/null || { echo 'ctest not available' >&2; exit 5; }
# Install Catch2 single-header v2.13.10 idempotently with sanity check
CATCH_DIR="/usr/local/include/catch2"
CATCH_HDR="$CATCH_DIR/catch.hpp"
if [ ! -f "$CATCH_HDR" ]; then
  sudo mkdir -p "$CATCH_DIR"
  TMP=$(mktemp)
  URL=https://raw.githubusercontent.com/catchorg/Catch2/v2.13.10/single_include/catch2/catch.hpp
  tries=0
  until curl -sSL -o "$TMP" "$URL"; do
    tries=$((tries+1))
    [ "$tries" -ge 3 ] && { echo 'failed to download Catch2' >&2; rm -f "$TMP"; exit 6; }
    sleep 1
  done
  # sanity check: ensure header contains expected tokens
  if ! grep -q "CATCH_CONFIG_MAIN\|CATCH_CONFIG_RUNNER" "$TMP"; then
    echo 'catch header missing expected token' >&2; rm -f "$TMP"; exit 7
  fi
  sudo mv "$TMP" "$CATCH_HDR"
fi
# final validation
command -v cmake >/dev/null || { echo 'cmake not on PATH after install' >&2; exit 8; }
command -v ctest >/dev/null || { echo 'ctest not on PATH after install' >&2; exit 9; }
# end
