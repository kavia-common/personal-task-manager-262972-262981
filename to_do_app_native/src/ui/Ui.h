#pragma once
#include <functional>
#include <optional>
#include <string>
#include <vector>

#ifdef APP_WITH_GUI
#include <SDL.h>
#endif

#include "model/TaskList.h"

/**
 * PUBLIC_INTERFACE
 * class Ui
 *
 * ImGui-based UI layer for the To-Do app. Renders header (new task inputs),
 * filters/search, task list with inline editing, and footer with counts and save.
 * Exposes callbacks for CRUD operations which the Application wires to the model.
 */
class Ui {
public:
    explicit Ui(TaskList& model);
    ~Ui() = default;

#ifdef APP_WITH_GUI
    // PUBLIC_INTERFACE
    void onEvent(const SDL_Event& e);
#endif

    // PUBLIC_INTERFACE
    void draw();

    // Callback setters
    // PUBLIC_INTERFACE
    void setOnCreateTask(std::function<void(const Task&)> fn);
    // PUBLIC_INTERFACE
    void setOnUpdateTask(std::function<void(const Task&)> fn);
    // PUBLIC_INTERFACE
    void setOnToggleComplete(std::function<void(uint64_t)> fn);
    // PUBLIC_INTERFACE
    void setOnDeleteTask(std::function<void(uint64_t)> fn);
    // PUBLIC_INTERFACE
    void setOnMove(std::function<void(uint64_t,int)> fn);
    // PUBLIC_INTERFACE
    void setOnSave(std::function<void()> fn);

private:
    TaskList& _model;

    // Input state
    std::string _newTitle;
    std::string _newDesc;
    std::string _newDue;

    TaskList::Filter _filter{TaskList::Filter::All};
    std::string _search;

    // Editing state
    std::optional<uint64_t> _editingId;
    std::string _editTitle;
    std::string _editDesc;
    std::string _editDue;

    // Callbacks
    std::function<void(const Task&)> _onCreate;
    std::function<void(const Task&)> _onUpdate;
    std::function<void(uint64_t)> _onToggle;
    std::function<void(uint64_t)> _onDelete;
    std::function<void(uint64_t,int)> _onMove;
    std::function<void()> _onSave;

    // helpers
    void drawHeader();
    void drawFilters();
    void drawTaskList();
    void drawFooter();

    void startEdit(const Task& t);
    void commitEdit();
    void cancelEdit();
};
