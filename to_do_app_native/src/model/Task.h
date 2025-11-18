#pragma once
#include <string>
#include <cstdint>

/**
 * PUBLIC_INTERFACE
 * struct Task
 *
 * Represents a single to-do item with validation semantics enforced by TaskList.
 */
struct Task {
    uint64_t id{0};
    std::string title;
    std::string description;
    std::string dueDate;
    bool completed{false};
    std::string createdAt;
    std::string updatedAt;
};

/**
 * PUBLIC_INTERFACE
 * std::string nowIso8601UTC()
 * Utility to get current UTC time as ISO8601 string (YYYY-MM-DDTHH:MM:SSZ).
 */
std::string nowIso8601UTC();

/**
 * PUBLIC_INTERFACE
 * std::string trim(const std::string& s)
 * Returns a copy of s with leading/trailing whitespace removed.
 */
std::string trim(const std::string& s);
