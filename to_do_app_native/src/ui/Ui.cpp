#include "ui/Ui.h"
#include "imgui.h"
#include <algorithm>

Ui::Ui(TaskList& model) : _model(model) {}

#ifdef APP_WITH_GUI
void Ui::onEvent(const SDL_Event& e) {
    // Keyboard shortcuts
    if (e.type == SDL_KEYDOWN) {
        const SDL_Keycode key = e.key.keysym.sym;
        const SDL_Keymod mod = (SDL_Keymod)SDL_GetModState();
        const bool ctrl = (mod & KMOD_CTRL) != 0;
        if (ctrl && (key == SDLK_s)) {
            if (_onSave) _onSave();
        }
        if (ctrl && (key == SDLK_f)) {
            // Focus search box next frame
            ImGui::SetNextItemFocus();
        }
        if (key == SDLK_RETURN && ImGui::GetIO().WantTextInput) {
            // Enter while typing in title adds task if in new title field
            // (Handled in draw by ImGui::IsItemDeactivatedAfterEdit on title box)
        }
        if (key == SDLK_DELETE) {
            if (_editingId.has_value()) {
                if (_onDelete) _onDelete(_editingId.value());
                _editingId.reset();
            }
        }
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
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("To-Do", nullptr, flags)) {
        drawHeader();
        ImGui::Separator();
        drawFilters();
        ImGui::Separator();
        drawTaskList();
        ImGui::Separator();
        drawFooter();
    }
    ImGui::End();
}

void Ui::drawHeader() {
    ImGui::TextColored(ImVec4(0.145f, 0.388f, 0.922f, 1.0f), "Add Task");
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##newTitle", &_newTitle, ImGuiInputTextFlags_EnterReturnsTrue)) {
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
    ImGui::InputTextMultiline("##newDesc", &_newDesc, ImVec2(-1, 80));
    ImGui::InputTextWithHint("##newDue", "Due date (optional)", &_newDue);
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
    ImGui::InputTextWithHint("##search", "Search title (Ctrl+F)", &_search);
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
            ImGui::InputText("##editTitle", &_editTitle);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(220);
            ImGui::InputTextWithHint("##editDue", "Due", &_editDue);
            ImGui::SameLine();
            if (ImGui::Button("Save")) { commitEdit(); }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) { cancelEdit(); }
            ImGui::InputTextMultiline("##editDesc", &_editDesc, ImVec2(-1, 80));
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
            if (ImGui::SmallButton("Delete")) { if (_onDelete) _onDelete(t.id); }
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
        for (const auto& t : _model.getTasks(TaskList::Filter::Completed, "")) {
            if (_onDelete) _onDelete(t.id);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Save (Ctrl+S)")) {
        if (_onSave) _onSave();
    }
}
