#pragma once
#include <optional>
#include <string>
#include <vector>
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

    // PUBLIC_INTERFACE
    bool importFromFile(const std::string& path, TaskList& outList, std::string& errMsg);

    // PUBLIC_INTERFACE
    bool exportToFile(const std::string& path, const TaskList& list, std::string& errMsg) const;

    // PUBLIC_INTERFACE
    std::string dataDir() const { return _dataDir; }

    // PUBLIC_INTERFACE
    std::string dataFile() const { return _dataFile; }

    // PUBLIC_INTERFACE
    std::string settingsFile() const { return _settingsFile; }

    // PUBLIC_INTERFACE
    static bool fileExists(const std::string& path);

private:
    std::string _dataDir;
    std::string _dataFile;
    std::string _settingsFile;
    int _backupKeep{3};

    static std::string detectDataDir();
    static bool ensureDir(const std::string& path);
    static bool atomicWrite(const std::string& file, const std::string& content);

    bool rotateBackups(const std::string& baseFile, int keep);
    bool tryParseTasksJson(const std::string& content, TaskList& out, std::string& err);
    std::optional<TaskList> loadFromPathWithRecovery(const std::string& primaryPath, int backupKeep, std::string& recoveryMsg);
};
