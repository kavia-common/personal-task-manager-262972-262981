#include "ui/Ui.h"
#include "imgui.h"
#include <algorithm>

// Helper: minimal wrappers to let ImGui edit std::string safely using internal buffers.
// These mirror the pattern from ImGui's FAQ (string input helpers).
static bool InputTextString(const char* label, std::string& str, ImGuiInputTextFlags flags = 0)
{
    // Allocate a buffer sized to current string + growth
    std::vector<char> buf(str.begin(), str.end());
    buf.push_back('\0');
    // Reserve some extra to reduce reallocs on small growth
    buf.reserve(std::max<size_t>(64, buf.size() + 64));
    bool changed = ImGui::InputText(label, buf.data(), buf.capacity(), flags);
    if (changed)
        str = std::string(buf.data());
    return changed;
}

static bool InputTextMultilineString(const char* label, std::string& str, const ImVec2& size = ImVec2(0,0), ImGuiInputTextFlags flags = 0)
{
    std::vector<char> buf(str.begin(), str.end());
    buf.push_back('\0');
    buf.reserve(std::max<size_t>(256, buf.size() + 128));
    bool changed = ImGui::InputTextMultiline(label, buf.data(), buf.capacity(), size, flags);
    if (changed)
        str = std::string(buf.data());
    return changed;
}

static bool InputTextWithHintString(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags = 0)
{
    std::vector<char> buf(str.begin(), str.end());
    buf.push_back('\0');
    buf.reserve(std::max<size_t>(64, buf.size() + 64));
    bool changed = ImGui::InputTextWithHint(label, hint, buf.data(), buf.capacity(), flags);
    if (changed)
        str = std::string(buf.data());
    return changed;
}

Ui::Ui(TaskList& model) : _model(model) {}

#ifdef APP_WITH_GUI
void Ui::onEvent(const SDL_Event& e) {
    // Keyboard shortcuts
    if (e.type == SDL_KEYDOWN) {
        const SDL_Keycode key = e.key.keysym.sym;
        const SDL_Keymod mod = (SDL_Keymod)SDL_GetModState();
        const bool ctrl = (mod & KMOD_CTRL) != 0;
        if (ctrl && (key == SDLK_s)) {
            if (_onSave) { _onSave(); _showSavedToast = true; _savedToastTimer = 1.0f; }
        }
        if (ctrl && (key == SDLK_f)) {
            _focusSearch = true;
        }
        // Enter handled by InputText with EnterReturnsTrue in drawHeader()
    }
}
#endif

void Ui::setOnCreateTask(std::function<void(const Task&)> fn) { _onCreate = std::move(fn); }
void Ui::setOnUpdateTask(std::function<void(const Task&)> fn) { _onUpdate = std::move(fn); }
void Ui::setOnToggleComplete(std::function<void(uint64_t)> fn){ _onToggle = std::move(fn); }
void Ui::setOnDeleteTask(std::function<void(uint64_t)> fn){ _onDelete = std::move(fn); }
void Ui::setOnMove(std::function<void(uint64_t,int)> fn){ _onMove = std::move(fn); }
void Ui::setOnSave(std::function<void()> fn){ _onSave = std::move(fn); }

void Ui::draw() {
    ImGui::SetNextWindowSize(ImVec2(960, 600), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin("To-Do", nullptr, flags)) {
        drawMenuBar();
        ImGui::Separator();
        drawHeader();
        ImGui::Separator();
        drawFilters();
        ImGui::Separator();
        drawTaskList();
        ImGui::Separator();
        drawFooter();
        drawModals();
        drawPathDialog();

        // Saved toast indicator
        if (_showSavedToast) {
            ImGui::SetNextWindowBgAlpha(0.85f);
            ImGui::Begin("##toast", nullptr,
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoInputs);
            ImGui::TextColored(ImVec4(0.145f, 0.388f, 0.922f, 1.0f), "Saved");
            ImGui::End();
            _savedToastTimer -= ImGui::GetIO().DeltaTime;
            if (_savedToastTimer <= 0.0f) _showSavedToast = false;
        }
    }
    ImGui::End();
}

void Ui::drawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Import...")) {
                _isExport = false;
                _pathDialog.clear();
                ImGui::OpenPopup("PathDialog");
            }
            if (ImGui::MenuItem("Export...")) {
                _isExport = true;
                _pathDialog.clear();
                ImGui::OpenPopup("PathDialog");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (_onSave) { _onSave(); _showSavedToast = true; _savedToastTimer = 1.0f; }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void Ui::drawHeader() {
    ImGui::TextColored(ImVec4(0.145f, 0.388f, 0.922f, 1.0f), "Add Task");
    ImGui::PushItemWidth(-1);
    if (InputTextString("##newTitle", _newTitle, ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (!_newTitle.empty()) {
            Task t;
            t.title = _newTitle;
            t.description = _newDesc;
            t.dueDate = _newDue;
            if (_onCreate) _onCreate(t);
            _newTitle.clear(); _newDesc.clear(); _newDue.clear();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add")) {
        if (!_newTitle.empty()) {
            Task t;
            t.title = _newTitle;
            t.description = _newDesc;
            t.dueDate = _newDue;
            if (_onCreate) _onCreate(t);
            _newTitle.clear(); _newDesc.clear(); _newDue.clear();
        }
    }
    InputTextMultilineString("##newDesc", _newDesc, ImVec2(-1, 80));
    InputTextWithHintString("##newDue", "Due date (optional)", _newDue);
    ImGui::PopItemWidth();
}

void Ui::drawFilters() {
    ImGui::TextColored(ImVec4(0.960f, 0.619f, 0.043f, 1.0f), "Filters");
    if (ImGui::RadioButton("All", _filter == TaskList::Filter::All)) _filter = TaskList::Filter::All;
    ImGui::SameLine();
    if (ImGui::RadioButton("Active", _filter == TaskList::Filter::Active)) _filter = TaskList::Filter::Active;
    ImGui::SameLine();
    if (ImGui::RadioButton("Completed", _filter == TaskList::Filter::Completed)) _filter = TaskList::Filter::Completed;
    ImGui::SameLine();
    ImGui::TextUnformatted("|");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(240);
    if (_focusSearch) { ImGui::SetKeyboardFocusHere(); _focusSearch = false; }
    InputTextWithHintString("##search", "Search title (Ctrl+F)", _search);
}

void Ui::startEdit(const Task& t) {
    _editingId = t.id;
    _editTitle = t.title;
    _editDesc = t.description;
    _editDue = t.dueDate;
}

void Ui::commitEdit() {
    if (!_editingId.has_value()) return;
    Task t;
    t.id = _editingId.value();
    t.title = _editTitle;
    t.description = _editDesc;
    t.dueDate = _editDue;
    if (_onUpdate) _onUpdate(t);
    _editingId.reset();
}

void Ui::cancelEdit() {
    _editingId.reset();
}

void Ui::drawTaskList() {
    auto items = _model.getTasks(_filter, _search);
    ImGui::BeginChild("task_list", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    for (const auto& t : items) {
        ImGui::PushID((int)t.id);
        ImGui::Separator();

        bool completed = t.completed;
        if (ImGui::Checkbox("##completed", &completed)) {
            if (_onToggle) _onToggle(t.id);
        }
        ImGui::SameLine();

        if (_editingId.has_value() && _editingId.value() == t.id) {
            ImGui::PushItemWidth(320);
            InputTextString("##editTitle", _editTitle);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(220);
            InputTextWithHintString("##editDue", "Due", _editDue);
            ImGui::SameLine();
            if (ImGui::Button("Save")) { commitEdit(); }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { cancelEdit(); }
            InputTextMultilineString("##editDesc", _editDesc, ImVec2(-1, 80));
        } else {
            ImGui::TextWrapped("%s", t.title.c_str());
            if (!t.dueDate.empty()) {
                ImGui::SameLine();
                ImGui::TextDisabled("(Due: %s)", t.dueDate.c_str());
            }
            if (!t.description.empty()) {
                ImGui::TextDisabled("%s", t.description.c_str());
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Edit")) { startEdit(t); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Delete")) { 
                _showConfirmDelete = true; 
                _pendingDeleteId = t.id; 
                ImGui::OpenPopup("ConfirmDelete"); 
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Up")) { if (_onMove) _onMove(t.id, -1); }
            ImGui::SameLine();
            if (ImGui::SmallButton("Down")) { if (_onMove) _onMove(t.id, +1); }
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void Ui::drawFooter() {
    const auto allCount = _model.getTasks(TaskList::Filter::All, "").size();
    const auto activeCount = _model.getTasks(TaskList::Filter::Active, "").size();
    const auto completedCount = _model.getTasks(TaskList::Filter::Completed, "").size();
    ImGui::Text("Total: %zu | Active: %zu | Completed: %zu", allCount, activeCount, completedCount);
    ImGui::SameLine();
    if (ImGui::Button("Clear Completed")) {
        _showConfirmClearCompleted = true;
        ImGui::OpenPopup("ConfirmClearCompleted");
    }
    ImGui::SameLine();
    if (ImGui::Button("Save (Ctrl+S)")) {
        if (_onSave) { _onSave(); _showSavedToast = true; _savedToastTimer = 1.0f; }
    }
}

void Ui::drawModals() {
    if (ImGui::BeginPopupModal("ConfirmDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete task? This cannot be undone.");
        if (ImGui::Button("Delete")) {
            if (_onDelete) _onDelete(_pendingDeleteId);
            _pendingDeleteId = 0;
            _showConfirmDelete = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            _pendingDeleteId = 0;
            _showConfirmDelete = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("ConfirmClearCompleted", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Remove all completed tasks? This cannot be undone.");
        if (ImGui::Button("Clear")) {
            if (_onClearCompleted) _onClearCompleted();
            _showConfirmClearCompleted = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            _showConfirmClearCompleted = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void Ui::drawPathDialog() {
    if (ImGui::BeginPopupModal("PathDialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(_isExport ? "Export to path" : "Import from path");
        ImGui::SetNextItemWidth(360);
        InputTextWithHintString("##path", _isExport ? "Enter export path..." : "Enter import path...", _pathDialog);
        if (ImGui::Button(_isExport ? "Export" : "Import")) {
            std::string toast;
            bool ok = false;
            if (_isExport && _onExport) ok = _onExport(_pathDialog, toast);
            if (!_isExport && _onImport) ok = _onImport(_pathDialog, toast);
            // Simple feedback via a temporary tooltip-like window
            ImGui::CloseCurrentPopup();
            if (!toast.empty()) {
                // Show a transient status in title bar area
                _showSavedToast = true;
                _savedToastTimer = 1.2f;
            }
            (void)ok;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
