#include "app/Application.h"
#include <iostream>

Application::Application() = default;
Application::~Application() = default;

bool Application::initialize() {
    _storage = std::make_unique<Storage>();
    _ui = std::make_unique<Ui>(_tasks);

    // Connect UI signals/actions to model changes and persistence
    _ui->setOnCreateTask([this](const Task& t) {
        try {
            _tasks.createTask(t.title, t.description, t.dueDate);
            markDirty();
        } catch (const std::exception& ex) {
            std::cerr << "CreateTask error: " << ex.what() << std::endl;
        }
    });

    _ui->setOnUpdateTask([this](const Task& t) {
        try {
            _tasks.updateTask(t);
            markDirty();
        } catch (const std::exception& ex) {
            std::cerr << "UpdateTask error: " << ex.what() << std::endl;
        }
    });

    _ui->setOnToggleComplete([this](uint64_t id) {
        if (_tasks.toggleComplete(id)) {
            markDirty();
        }
    });

    _ui->setOnDeleteTask([this](uint64_t id) {
        if (_tasks.deleteTask(id)) {
            markDirty();
        }
    });

    _ui->setOnMove([this](uint64_t id, int dir) {
        bool ok = false;
        if (dir < 0) ok = _tasks.moveUp(id);
        else if (dir > 0) ok = _tasks.moveDown(id);
        if (ok) markDirty();
    });

    _ui->setOnSave([this]() {
        save();
    });

    return true;
}

void Application::load() {
    auto loaded = _storage->loadTasks();
    if (loaded.has_value()) {
        _tasks = std::move(loaded.value());
    }
    _dirty = false;
}

bool Application::save() {
    if (_storage->saveTasks(_tasks)) {
        _dirty = false;
        return true;
    }
    return false;
}

void Application::render() {
    _ui->draw();
}

#ifdef APP_WITH_GUI
void Application::onEvent(const SDL_Event& e) {
    _ui->onEvent(e);
}
#endif

void Application::markDirty() {
    _dirty = true;
    _lastChange = std::chrono::steady_clock::now();
}

void Application::maybeAutoSave() {
    if (!_dirty) return;
    auto now = std::chrono::steady_clock::now();
    if (now - _lastChange >= _debounce) {
        save();
    }
}

size_t Application::taskCount() const {
    return _tasks.getTasks(TaskList::Filter::All, "").size();
}
