#include "persistence/Storage.h"
#include "persistence/Json.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <cerrno>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h>
#endif

using nlohmann::json;

static bool fileExists(const std::string& path) {
    struct stat buffer{};
    return (stat(path.c_str(), &buffer) == 0);
}

Storage::Storage() {
    _dataDir = detectDataDir();
    _dataFile = _dataDir + "/tasks.json";
}

std::string Storage::detectDataDir() {
#if defined(_WIN32)
    char* appdata = std::getenv("APPDATA");
    std::string base = appdata ? std::string(appdata) : ".";
    return base + "\\to_do_app_native";
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    std::string base = home ? std::string(home) : ".";
    return base + "/Library/Application Support/to_do_app_native";
#else
    const char* home = std::getenv("HOME");
    std::string base = home ? std::string(home) : ".";
    return base + "/.local/share/to_do_app_native";
#endif
}

bool Storage::ensureDir(const std::string& path) {
#if defined(_WIN32)
    // Create nested directories
    std::string partial;
    for (char c : path) {
        partial.push_back(c);
        if (c == '\\' || c == '/') {
            CreateDirectoryA(partial.c_str(), NULL);
        }
    }
    if (!CreateDirectoryA(path.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }
    return true;
#else
    // mkdir -p like
    std::string partial;
    for (size_t i = 0; i < path.size(); ++i) {
        partial.push_back(path[i]);
        if (path[i] == '/') {
            mkdir(partial.c_str(), 0755);
        }
    }
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        return false;
    }
    return true;
#endif
}

bool Storage::atomicWrite(const std::string& file, const std::string& content) {
    std::string tmp = file + ".tmp";
    {
        std::ofstream ofs(tmp, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open()) return false;
        ofs.write(content.data(), (std::streamsize)content.size());
        ofs.flush();
        if (!ofs.good()) return false;
    }
#if defined(_WIN32)
    // ReplaceFileA requires backup name; use MoveFileEx as atomic enough on NTFS with replace existing
    if (!MoveFileExA(tmp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        return false;
    }
#else
    if (std::rename(tmp.c_str(), file.c_str()) != 0) {
        return false;
    }
#endif
    return true;
}

std::optional<TaskList> Storage::loadTasks() {
    TaskList list;
    if (!fileExists(_dataFile)) {
        ensureDir(_dataDir);
        return list;
    }
    std::ifstream ifs(_dataFile);
    if (!ifs.is_open()) {
        return list;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    try {
        json j = json::parse(buffer.str());
        if (!j.is_object()) throw std::runtime_error("Root not object");
        if (!j.contains("tasks") || !j["tasks"].is_array()) {
            throw std::runtime_error("Missing tasks array");
        }
        std::vector<Task> tasks;
        for (const auto& jt : j["tasks"]) {
            Task t;
            t.id = jt.value("id", 0ull);
            t.title = jt.value("title", std::string{});
            t.description = jt.value("description", std::string{});
            t.dueDate = jt.value("dueDate", std::string{});
            t.completed = jt.value("completed", false);
            t.createdAt = jt.value("createdAt", std::string{});
            t.updatedAt = jt.value("updatedAt", std::string{});
            tasks.push_back(std::move(t));
        }
        list.setAll(std::move(tasks));
        return list;
    } catch (...) {
        // Backup malformed file
        std::string bak = _dataFile + ".bak";
        std::ifstream src(_dataFile, std::ios::binary);
        std::ofstream dst(bak, std::ios::binary | std::ios::trunc);
        dst << src.rdbuf();
        return TaskList{};
    }
}

bool Storage::saveTasks(const TaskList& list) {
    ensureDir(_dataDir);
    json j;
    j["tasks"] = json::array();
    for (const auto& t : list.all()) {
        json jt;
        jt["id"] = t.id;
        jt["title"] = t.title;
        jt["description"] = t.description;
        jt["dueDate"] = t.dueDate;
        jt["completed"] = t.completed;
        jt["createdAt"] = t.createdAt;
        jt["updatedAt"] = t.updatedAt;
        j["tasks"].push_back(std::move(jt));
    }
    std::string content = j.dump(2);
    return atomicWrite(_dataFile, content);
}
