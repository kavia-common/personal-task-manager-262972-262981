#pragma once
#include <memory>
#include <string>
#include <chrono>

#ifdef APP_WITH_GUI
#include <SDL.h>
#endif

#include "ui/Ui.h"
#include "model/TaskList.h"
#include "persistence/Storage.h"

/**
 * PUBLIC_INTERFACE
 * class Application
 *
 * High-level application controller. Manages lifecycle, model, persistence,
 * event handling, and triggers UI rendering. Provides auto-save debounce
 * after changes (~500ms).
 */
class Application {
public:
    Application();
    ~Application();

    // Initialize subsystems
    // PUBLIC_INTERFACE
    bool initialize();

    // PUBLIC_INTERFACE
    void load();

    // PUBLIC_INTERFACE
    bool save();

    // PUBLIC_INTERFACE
    void render();

#ifdef APP_WITH_GUI
    // PUBLIC_INTERFACE
    void onEvent(const SDL_Event& e);
#endif

    // PUBLIC_INTERFACE
    void markDirty();

    // PUBLIC_INTERFACE
    void maybeAutoSave();

    // PUBLIC_INTERFACE
    size_t taskCount() const;

private:
    std::unique_ptr<Ui> _ui;
    std::unique_ptr<Storage> _storage;
    TaskList _tasks;

    bool _dirty{false};
    std::chrono::steady_clock::time_point _lastChange{};
    std::chrono::milliseconds _debounce{500};
};
