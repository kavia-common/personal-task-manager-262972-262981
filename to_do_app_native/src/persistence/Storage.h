#pragma once
#include <optional>
#include <string>
#include "model/TaskList.h"

/**
 * PUBLIC_INTERFACE
 * class Storage
 *
 * Handles cross-platform JSON persistence with atomic saves and error recovery.
 * Paths:
 *  - Linux: ~/.local/share/to_do_app_native/tasks.json
 *  - macOS: ~/Library/Application Support/to_do_app_native/tasks.json
 *  - Windows: %APPDATA%/to_do_app_native/tasks.json
 */
class Storage {
public:
    Storage();
    ~Storage() = default;

    // PUBLIC_INTERFACE
    std::optional<TaskList> loadTasks();

    // PUBLIC_INTERFACE
    bool saveTasks(const TaskList& list);

private:
    std::string _dataDir;
    std::string _dataFile;

    static std::string detectDataDir();
    static bool ensureDir(const std::string& path);
    static bool atomicWrite(const std::string& file, const std::string& content);
};
