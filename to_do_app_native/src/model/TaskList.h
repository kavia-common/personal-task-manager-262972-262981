#pragma once
#include <vector>
#include <optional>
#include <string>
#include <cstdint>
#include "model/Task.h"

/**
 * PUBLIC_INTERFACE
 * class TaskList
 *
 * Manages a collection of Task with CRUD operations, filtering and search.
 * Titles are validated (trimmed, non-empty, <= 200 chars).
 */
class TaskList {
public:
    enum class Filter { All, Active, Completed };

    TaskList();

    // PUBLIC_INTERFACE
    const std::vector<Task>& all() const;

    // PUBLIC_INTERFACE
    std::vector<Task> getTasks(Filter filter, const std::string& search) const;

    // PUBLIC_INTERFACE
    Task& createTask(const std::string& title, const std::string& description, const std::string& dueDate);

    // PUBLIC_INTERFACE
    bool updateTask(const Task& updated);

    // PUBLIC_INTERFACE
    bool toggleComplete(uint64_t id);

    // PUBLIC_INTERFACE
    bool deleteTask(uint64_t id);

    // PUBLIC_INTERFACE
    bool moveUp(uint64_t id);

    // PUBLIC_INTERFACE
    bool moveDown(uint64_t id);

    // PUBLIC_INTERFACE
    void clear();

    // For persistence
    // PUBLIC_INTERFACE
    void setAll(std::vector<Task> tasks);

private:
    std::vector<Task> _items;
    uint64_t _nextId{1};

    static void validateTitle(const std::string& title);
    int indexOf(uint64_t id) const;
};
