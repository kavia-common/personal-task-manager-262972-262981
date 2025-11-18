#include "model/TaskList.h"
#include <stdexcept>
#include <algorithm>

TaskList::TaskList() = default;

const std::vector<Task>& TaskList::all() const { return _items; }

std::vector<Task> TaskList::getTasks(Filter filter, const std::string& search) const {
    std::vector<Task> out;
    std::string q = trim(search);
    std::transform(q.begin(), q.end(), q.begin(), ::tolower);
    for (const auto& t : _items) {
        if (filter == Filter::Active && t.completed) continue;
        if (filter == Filter::Completed && !t.completed) continue;
        if (!q.empty()) {
            std::string titleLower = t.title;
            std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            if (titleLower.find(q) == std::string::npos) continue;
        }
        out.push_back(t);
    }
    return out;
}

void TaskList::validateTitle(const std::string& title) {
    auto v = trim(title);
    if (v.empty()) throw std::invalid_argument("Title cannot be empty");
    if (v.size() > 200) throw std::invalid_argument("Title too long (max 200)");
}

Task& TaskList::createTask(const std::string& title, const std::string& description, const std::string& dueDate) {
    validateTitle(title);
    Task t;
    t.id = _nextId++;
    t.title = trim(title);
    t.description = description; // allow newlines
    t.dueDate = trim(dueDate);
    t.completed = false;
    t.createdAt = nowIso8601UTC();
    t.updatedAt = t.createdAt;
    _items.push_back(t);
    return _items.back();
}

bool TaskList::updateTask(const Task& updated) {
    int idx = indexOf(updated.id);
    if (idx < 0) return false;
    validateTitle(updated.title);
    _items[idx].title = trim(updated.title);
    _items[idx].description = updated.description;
    _items[idx].dueDate = trim(updated.dueDate);
    _items[idx].updatedAt = nowIso8601UTC();
    return true;
}

bool TaskList::toggleComplete(uint64_t id) {
    int idx = indexOf(id);
    if (idx < 0) return false;
    _items[idx].completed = !_items[idx].completed;
    _items[idx].updatedAt = nowIso8601UTC();
    return true;
}

bool TaskList::deleteTask(uint64_t id) {
    int idx = indexOf(id);
    if (idx < 0) return false;
    _items.erase(_items.begin() + idx);
    return true;
}

bool TaskList::moveUp(uint64_t id) {
    int idx = indexOf(id);
    if (idx <= 0) return false;
    std::swap(_items[idx-1], _items[idx]);
    return true;
}

bool TaskList::moveDown(uint64_t id) {
    int idx = indexOf(id);
    if (idx < 0 || idx >= (int)_items.size()-1) return false;
    std::swap(_items[idx+1], _items[idx]);
    return true;
}

void TaskList::clear() {
    _items.clear();
    _nextId = 1;
}

void TaskList::setAll(std::vector<Task> tasks) {
    _items = std::move(tasks);
    // Recompute nextId
    uint64_t maxId = 0;
    for (const auto& t : _items) maxId = std::max(maxId, t.id);
    _nextId = maxId + 1;
}

int TaskList::indexOf(uint64_t id) const {
    for (size_t i=0;i<_items.size();++i) {
        if (_items[i].id == id) return (int)i;
    }
    return -1;
}
