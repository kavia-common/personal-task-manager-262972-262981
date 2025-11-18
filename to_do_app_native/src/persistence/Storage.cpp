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

bool Storage::fileExists(const std::string& path) {
    struct stat buffer{};
    return (stat(path.c_str(), &buffer) == 0);
}

Storage::Storage() {
    _dataDir = detectDataDir();
    _dataFile = _dataDir + "/tasks.json";
    _settingsFile = _dataDir + "/settings.json";
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

bool Storage::rotateBackups(const std::string& baseFile, int keep) {
    // Rotate: base.bak{keep-1} -> base.bak{keep}, ..., base.bak1 -> base.bak2, base -> base.bak1
    if (keep < 1) return true;
    for (int i = keep; i >= 2; --i) {
        std::string older = baseFile + ".bak" + std::to_string(i - 1);
        std::string newer = baseFile + ".bak" + std::to_string(i);
        if (fileExists(older)) {
#if defined(_WIN32)
            MoveFileExA(older.c_str(), newer.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
#else
            std::rename(older.c_str(), newer.c_str());
#endif
        }
    }
    // Move current .bak1 target from base file
    if (fileExists(baseFile)) {
        std::ifstream src(baseFile, std::ios::binary);
        std::ofstream dst(baseFile + ".bak1", std::ios::binary | std::ios::trunc);
        if (!src.is_open() || !dst.is_open()) return false;
        dst << src.rdbuf();
    }
    return true;
}

bool Storage::tryParseTasksJson(const std::string& content, TaskList& out, std::string& err) {
    try {
        json j = json::parse(content);
        if (!j.is_object()) { err = "Root is not an object"; return false; }
        if (!j.contains("tasks") || !j["tasks"].is_array()) { err = "Missing tasks[]"; return false; }
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
        out.setAll(std::move(tasks));
        return true;
    } catch (const std::exception& ex) {
        err = ex.what();
        return false;
    } catch (...) {
        err = "Unknown parse error";
        return false;
    }
}

std::optional<TaskList> Storage::loadFromPathWithRecovery(const std::string& primaryPath, int backupKeep, std::string& recoveryMsg) {
    TaskList list;
    ensureDir(_dataDir);
    if (!fileExists(primaryPath)) return list;
    std::ifstream ifs(primaryPath, std::ios::binary);
    if (!ifs.is_open()) return list;
    std::stringstream buf; buf << ifs.rdbuf();
    std::string err;
    if (tryParseTasksJson(buf.str(), list, err)) {
        return list;
    }
    // Primary failed; try backups .bak1..bakKeep
    recoveryMsg = "Primary tasks.json invalid, attempting recovery from backups.";
    for (int i = 1; i <= backupKeep; ++i) {
        std::string bak = primaryPath + ".bak" + std::to_string(i);
        if (!fileExists(bak)) continue;
        std::ifstream bifs(bak, std::ios::binary);
        if (!bifs.is_open()) continue;
        std::stringstream bbuf; bbuf << bifs.rdbuf();
        TaskList recovered;
        std::string berr;
        if (tryParseTasksJson(bbuf.str(), recovered, berr)) {
            return recovered;
        }
    }
    // As a last resort, create a single non-rotated .bak copy of the corrupted file for user inspection.
    std::ifstream src(primaryPath, std::ios::binary);
    std::ofstream dst(primaryPath + ".bak_corrupt", std::ios::binary | std::ios::trunc);
    if (src.is_open() && dst.is_open()) dst << src.rdbuf();
    return TaskList{};
}

std::optional<TaskList> Storage::loadTasks() {
    std::string recMsg;
    auto res = loadFromPathWithRecovery(_dataFile, _backupKeep, recMsg);
    return res;
}

bool Storage::saveTasks(const TaskList& list) {
    ensureDir(_dataDir);
    // Rotate backups first
    rotateBackups(_dataFile, _backupKeep);

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

bool Storage::importFromFile(const std::string& path, TaskList& outList, std::string& errMsg) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) { errMsg = "Cannot open file"; return false; }
    std::stringstream buf; buf << ifs.rdbuf();
    TaskList parsed;
    if (!tryParseTasksJson(buf.str(), parsed, errMsg)) {
        return false;
    }
    outList = std::move(parsed);
    return true;
}

bool Storage::exportToFile(const std::string& path, const TaskList& list, std::string& errMsg) const {
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
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs.is_open()) { errMsg = "Cannot write file"; return false; }
    ofs << content;
    ofs.flush();
    if (!ofs.good()) { errMsg = "Write failed"; return false; }
    return true;
}
