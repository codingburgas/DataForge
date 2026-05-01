#include "ui/dialogs.h"
#include "ui/toast.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    static const char* PRIORITY_ITEMS[] = {
        "Low", "Medium", "High", "Critical"
    };
    static const char* STATUS_ITEMS[] = {
        "To-do", "In progress", "Done", "Blocked"
    };

    static const char* POPUP_ADD_EDIT      = "Task";
    static const char* POPUP_CONFIRM_DEL   = "Confirm delete";
    static const char* POPUP_DIRTY_EXIT    = "Unsaved changes";
    static const char* POPUP_ABOUT         = "About DataForge";

    void openCreateDialog(UiState& uiState, int parentId) {
        clearEditBuffers(uiState.edit);
        uiState.edit.parentId         = parentId;
        uiState.editingTaskId         = 0;
        uiState.parentForNewTask      = parentId;
        uiState.showAddEditDialog     = true;
        uiState.lastValidationOk      = true;
        uiState.lastValidationMessage.clear();
    }

    void openEditDialog(UiState& uiState, const data::Task& t) {
        loadEditBuffersFromTask(uiState.edit, t);
        uiState.editingTaskId         = t.id;
        uiState.showAddEditDialog     = true;
        uiState.lastValidationOk      = true;
        uiState.lastValidationMessage.clear();
    }

    void openConfirmDelete(UiState& uiState, int taskId) {
        uiState.pendingDeleteId   = taskId;
        uiState.showConfirmDelete = true;
    }

    static void renderParentPicker(const data::TaskStore& store,
                                   UiState& uiState) {
        std::vector<int> candidates;
        candidates.push_back(-1);
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            int id = store.tasks[i].id;
            if (uiState.editingTaskId > 0 && id == uiState.editingTaskId) {
                continue;
            }
            candidates.push_back(id);
        }
        std::string preview;
        if (uiState.edit.parentId == -1) {
            preview = "(root)";
        } else {
            const data::Task* p = nullptr;
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].id == uiState.edit.parentId) {
                    p = &store.tasks[i];
                    break;
                }
            }
            preview = p != nullptr ? ("#" + std::to_string(p->id) + " " + p->title)
                                   : "(invalid)";
        }
        if (ImGui::BeginCombo("Parent", preview.c_str())) {
            for (std::size_t i = 0; i < candidates.size(); ++i) {
                int cid = candidates[i];
                std::string label;
                if (cid == -1) {
                    label = "(root)";
                } else {
                    for (std::size_t j = 0; j < store.tasks.size(); ++j) {
                        if (store.tasks[j].id == cid) {
                            label = "#" + std::to_string(cid) +
                                    " " + store.tasks[j].title;
                            break;
                        }
                    }
                }
                bool selected = (uiState.edit.parentId == cid);
                if (ImGui::Selectable(label.c_str(), selected)) {
                    uiState.edit.parentId = cid;
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

    static void renderForm(data::TaskStore& store, UiState& uiState) {
        ImGui::InputText("Title",
                         uiState.edit.titleBuf, TITLE_BUF_SIZE);
        ImGui::InputTextMultiline("Description",
                                  uiState.edit.descBuf, DESC_BUF_SIZE,
                                  ImVec2(-1.0f, 80.0f));
        ImGui::Combo("Priority",
                     &uiState.edit.priority,
                     PRIORITY_ITEMS, IM_ARRAYSIZE(PRIORITY_ITEMS));
        ImGui::Combo("Status",
                     &uiState.edit.status,
                     STATUS_ITEMS, IM_ARRAYSIZE(STATUS_ITEMS));
        ImGui::InputText("Deadline (YYYY-MM-DD)",
                         uiState.edit.deadlineBuf, DATE_BUF_SIZE);
        ImGui::InputText("Estimated minutes",
                         uiState.edit.estimateBuf, NUMERIC_BUF_SIZE,
                         ImGuiInputTextFlags_CharsDecimal);
        ImGui::InputText("Actual minutes",
                         uiState.edit.actualBuf, NUMERIC_BUF_SIZE,
                         ImGuiInputTextFlags_CharsDecimal);
        renderParentPicker(store, uiState);
    }

    void renderAddEditDialog(data::TaskStore& store, UiState& uiState) {
        if (uiState.showAddEditDialog) {
            ImGui::OpenPopup(POPUP_ADD_EDIT);
            uiState.showAddEditDialog = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(520.0f, 0.0f), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal(POPUP_ADD_EDIT, nullptr,
                                    ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        if (uiState.editingTaskId > 0) {
            ImGui::Text("Editing task #%d", uiState.editingTaskId);
        } else {
            ImGui::Text("New task");
        }
        ImGui::Separator();
        renderForm(store, uiState);
        if (!uiState.lastValidationOk && !uiState.lastValidationMessage.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f),
                               "%s", uiState.lastValidationMessage.c_str());
        }
        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Save", ImVec2(100.0f, 0.0f))) {
            data::Task t{};
            if (uiState.editingTaskId > 0) {
                for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                    if (store.tasks[i].id == uiState.editingTaskId) {
                        t = store.tasks[i];
                        break;
                    }
                }
            } else {
                t.id        = 0;
                t.createdAt = logic::zeroDate();
                t.updatedAt = logic::zeroDate();
            }
            applyEditBuffersToTask(uiState.edit, t);
            logic::ValidationResult vr;
            if (uiState.editingTaskId > 0) {
                t.id = uiState.editingTaskId;
                if (logic::editTask(store, t, vr)) {
                    pushToast(uiState, "Task updated.");
                    uiState.lastValidationOk = true;
                    uiState.lastValidationMessage.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    uiState.lastValidationOk      = vr.ok;
                    uiState.lastValidationMessage = vr.message;
                }
            } else {
                int newId = logic::createTask(store, t, vr);
                if (newId > 0) {
                    uiState.selectedTaskId = newId;
                    pushToast(uiState, "Task created.");
                    uiState.lastValidationOk = true;
                    uiState.lastValidationMessage.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    uiState.lastValidationOk      = vr.ok;
                    uiState.lastValidationMessage = vr.message;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
            uiState.lastValidationOk = true;
            uiState.lastValidationMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void renderConfirmDeleteDialog(data::TaskStore& store, UiState& uiState) {
        if (uiState.showConfirmDelete) {
            ImGui::OpenPopup(POPUP_CONFIRM_DEL);
            uiState.showConfirmDelete = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));
        if (!ImGui::BeginPopupModal(POPUP_CONFIRM_DEL, nullptr,
                                    ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        int id = uiState.pendingDeleteId;
        if (id <= 0) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }
        const data::Task* t = nullptr;
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == id) {
                t = &store.tasks[i];
                break;
            }
        }
        if (t == nullptr) {
            ImGui::Text("Task no longer exists.");
            if (ImGui::Button("OK")) {
                uiState.pendingDeleteId = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            return;
        }
        int descendants = logic::countDescendants(store, id);
        ImGui::Text("Delete task #%d \"%s\"?",
                    t->id, t->title.c_str());
        if (descendants > 0) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.95f, 0.65f, 0.3f, 1.0f),
                               "This will also delete %d subtask%s.",
                               descendants,
                               descendants == 1 ? "" : "s");
        }
        ImGui::Spacing();
        if (ImGui::Button("Delete", ImVec2(100.0f, 0.0f))) {
            logic::takeSnapshot(store, uiState.undoSnapshot);
            uiState.hasUndoSnapshot = true;
            int n = logic::deleteTaskCascade(store, id);
            std::string msg = "Deleted " + std::to_string(n) + " task";
            msg += (n == 1 ? "." : "s.");
            pushToast(uiState, msg);
            if (uiState.selectedTaskId == id) {
                uiState.selectedTaskId = -1;
            }
            uiState.pendingDeleteId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
            uiState.pendingDeleteId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void renderDirtyExitConfirmDialog(data::TaskStore& store,
                                      UiState& uiState) {
        if (uiState.showDirtyExitConfirm) {
            ImGui::OpenPopup(POPUP_DIRTY_EXIT);
            uiState.showDirtyExitConfirm = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));
        if (!ImGui::BeginPopupModal(POPUP_DIRTY_EXIT, nullptr,
                                    ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        ImGui::Text("There are unsaved changes.");
        ImGui::Spacing();
        if (ImGui::Button("Save and quit", ImVec2(140.0f, 0.0f))) {
            uiState.triggeredSave = true;
            uiState.requestedQuit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Quit without saving", ImVec2(160.0f, 0.0f))) {
            store.dirty           = false;
            uiState.requestedQuit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
            uiState.requestedQuit = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void renderAboutPopup(UiState& uiState) {
        if (uiState.showAboutPopup) {
            ImGui::OpenPopup(POPUP_ABOUT);
            uiState.showAboutPopup = false;
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));
        if (!ImGui::BeginPopupModal(POPUP_ABOUT, nullptr,
                                    ImGuiWindowFlags_AlwaysAutoResize)) {
            return;
        }
        ImGui::Text("DataForge");
        ImGui::Text("Hierarchical task manager in procedural C++.");
        ImGui::Separator();
        ImGui::Text("Dear ImGui + Win32 + DirectX 11 backend.");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120.0f, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

}
