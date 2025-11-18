#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
mkdir -p "$WS/src" "$WS/build" "$WS/cmake" "$WS/logs" && cd "$WS"
cat > "$WS/src/main.cpp" <<'CPP'
#include <gtk/gtk.h>
static void on_activate(GApplication *app, gpointer user_data){
  GtkWidget *win = gtk_application_window_new(GTK_APPLICATION(app));
  gtk_window_set_title(GTK_WINDOW(win), "ToDo - Scaffold");
  gtk_window_set_default_size(GTK_WINDOW(win), 480, 240);
  gtk_widget_show(win);
}
int main(int argc, char **argv){
  GtkApplication *app = gtk_application_new("org.example.ToDo", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
CPP
cat > "$WS/CMakeLists.txt" <<'CM'
cmake_minimum_required(VERSION 3.16)
project(to_do_app_native LANGUAGES CXX)
find_package(PkgConfig REQUIRED)
# Create imported target if pkg-config provides it
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
add_executable(to_do_app_native src/main.cpp)
# Link against the imported pkg-config target if available
if(TARGET PkgConfig::GTK4)
  target_link_libraries(to_do_app_native PRIVATE PkgConfig::GTK4)
else()
  if(GTK4_FOUND)
    target_include_directories(to_do_app_native PRIVATE ${GTK4_INCLUDE_DIRS})
    target_link_libraries(to_do_app_native PRIVATE ${GTK4_LIBRARIES})
    if(GTK4_CFLAGS_OTHER)
      target_compile_options(to_do_app_native PRIVATE ${GTK4_CFLAGS_OTHER})
    endif()
  else()
    message(FATAL_ERROR "GTK4 pkg-config not found; install libgtk-4-dev")
  endif()
endif()
# Optionally include tests fragment
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/tests.cmake")
  include(${CMAKE_SOURCE_DIR}/cmake/tests.cmake)
endif()
CM
cat > "$WS/build.sh" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
mkdir -p "$WS/build" && cd "$WS/build"
cmake -S "$WS" -B . -DCMAKE_BUILD_TYPE=Debug || { echo 'cmake configure failed' >&2; exit 3; }
cmake --build . -- -j"$(nproc)" || { echo 'build failed' >&2; exit 4; }
SH
chmod +x "$WS/build.sh"
cat > "$WS/start-wrapper.sh" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
WS="/home/kavia/workspace/code-generation/personal-task-manager-262972-262981/to_do_app_native"
BIN="$WS/build/to_do_app_native"
PIDFILE="$WS/.app_pid"
LOG="$WS/logs/app.log"
[ -x "$BIN" ] || { echo 'app binary missing or not executable' >&2; exit 2; }
printf "" > "$LOG"
# Write pidfile then exec the binary so the recorded PID is the long-lived process
sh -c 'printf "%s" "$$" > "'"$PIDFILE"'" && exec "'"$BIN"'"' > "$LOG" 2>&1 &
sleep 0.5
exit 0
SH
chmod +x "$WS/start-wrapper.sh"
