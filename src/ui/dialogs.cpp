#include "ui/dialogs.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "ui/voice_input.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    namespace {

        static const char* PRIORITY_ITEMS[] = { "Low", "Medium", "High", "Critical" };
        static const char* STATUS_ITEMS[]   = { "To Do", "In Progress", "Done", "Blocked" };

        static const char* POPUP_ADD_EDIT    = "Task";
        static const char* POPUP_CONFIRM_DEL = "Confirm Delete";
        static const char* POPUP_DIRTY_EXIT  = "Unsaved Changes";
        static const char* POPUP_ABOUT       = "About DataForge";

        void fieldLabel(const char* text) {
            ImGui::Dummy(ImVec2(1.0f, 6.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextMuted, "%s", text);
            ImGui::PopFont();
        }

        void renderParentPicker(const data::TaskStore& store, UiState& uiState) {
            std::vector<int> candidates;
            candidates.push_back(-1);
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                int id = store.tasks[i].id;
                if (uiState.editingTaskId > 0 && id == uiState.editingTaskId) {
                    continue;
                }
                candidates.push_back(id);
            }

            std::string preview = "(root)";
            if (uiState.edit.parentId != -1) {
                preview = "(invalid)";
                for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                    if (store.tasks[i].id == uiState.edit.parentId) {
                        preview = "#" + std::to_string(store.tasks[i].id) + " " + store.tasks[i].title;
                        break;
                    }
                }
            }

            if (ImGui::BeginCombo("##Parent", preview.c_str())) {
                for (std::size_t i = 0; i < candidates.size(); ++i) {
                    int cid = candidates[i];
                    std::string label = "(root)";
                    if (cid != -1) {
                        for (std::size_t j = 0; j < store.tasks.size(); ++j) {
                            if (store.tasks[j].id == cid) {
                                label = "#" + std::to_string(cid) + " " + store.tasks[j].title;
                                break;
                            }
                        }
                    }
                    bool selected = uiState.edit.parentId == cid;
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

        void renderForm(data::TaskStore& store, UiState& uiState, float formW) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ColBgSubtle);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, HEX(0xF4F7FB));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);

            const float micW = 48.0f;
            const float micGap = 6.0f;

            fieldLabel("Title");
            ImGui::SetNextItemWidth(formW - micW - micGap);
            ImGui::InputText("##Title", uiState.edit.titleBuf, TITLE_BUF_SIZE);
            ImGui::SameLine(0, micGap);
            voiceMicButton("micTitle", uiState.edit.titleBuf, TITLE_BUF_SIZE);

            fieldLabel("Description");
            ImGui::SetNextItemWidth(formW - micW - micGap);
            ImGui::InputTextMultiline("##Desc", uiState.edit.descBuf, DESC_BUF_SIZE,
                                      ImVec2(formW - micW - micGap, 110.0f));
            ImGui::SameLine(0, micGap);
            voiceMicButton("micDesc", uiState.edit.descBuf, DESC_BUF_SIZE, 110.0f);

            fieldLabel("Priority");
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::Combo("##Priority", &uiState.edit.priority,
                         PRIORITY_ITEMS, IM_ARRAYSIZE(PRIORITY_ITEMS));
            ImGui::SameLine(0, 12.0f);
            fieldLabel("Status");
            ImGui::SetCursorPosX(ImGui::GetCursorPosX());
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::Combo("##Status", &uiState.edit.status,
                         STATUS_ITEMS, IM_ARRAYSIZE(STATUS_ITEMS));

            fieldLabel("Deadline (YYYY-MM-DD)");
            ImGui::SetNextItemWidth(formW);
            ImGui::InputText("##Deadline", uiState.edit.deadlineBuf, DATE_BUF_SIZE);

            fieldLabel("Estimated minutes");
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::InputText("##EstMins", uiState.edit.estimateBuf, NUMERIC_BUF_SIZE,
                             ImGuiInputTextFlags_CharsDecimal);
            ImGui::SameLine(0, 12.0f);
            fieldLabel("Actual minutes");
            ImGui::SetCursorPosX(ImGui::GetCursorPosX());
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::InputText("##ActMins", uiState.edit.actualBuf, NUMERIC_BUF_SIZE,
                             ImGuiInputTextFlags_CharsDecimal);

            fieldLabel("Parent task");
            ImGui::SetNextItemWidth(formW);
            renderParentPicker(store, uiState);

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
        }

        void setupModal(ImVec2 size) {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(size, ImGuiCond_Always);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 22.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ColBgCard);
        }

        void endModalSetup() {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
        }

    }

    void openCreateDialog(UiState& uiState, int parentId) {
        clearEditBuffers(uiState.edit);
        uiState.edit.parentId     = parentId;
        uiState.editingTaskId     = 0;
        uiState.parentForNewTask  = parentId;
        uiState.showAddEditDialog = true;
        uiState.lastValidationOk  = true;
        uiState.lastValidationMessage.clear();
    }

    void openEditDialog(UiState& uiState, const data::Task& task) {
        loadEditBuffersFromTask(uiState.edit, task);
        uiState.editingTaskId     = task.id;
        uiState.showAddEditDialog = true;
        uiState.lastValidationOk  = true;
        uiState.lastValidationMessage.clear();
    }

    void openConfirmDelete(UiState& uiState, int taskId) {
        uiState.pendingDeleteId   = taskId;
        uiState.showConfirmDelete = true;
    }

    void renderAddEditDialog(data::TaskStore& store, UiState& uiState) {
        if (uiState.showAddEditDialog) {
            ImGui::OpenPopup(POPUP_ADD_EDIT);
            uiState.showAddEditDialog = false;
        }
        setupModal(ImVec2(680.0f, 0.0f));

        if (!ImGui::BeginPopupModal(POPUP_ADD_EDIT, nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            endModalSetup();
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetWindowPos();
        float width = ImGui::GetWindowWidth();
        drawGradientRect(dl, min, ImVec2(min.x + width, min.y + 6.0f), 22.0f);

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary,
                           "%s", uiState.editingTaskId > 0 ? "Edit Task" : "Create Task");
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "Capture the work clearly so the queue stays readable.");
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 10.0f));

        float formW = width - 48.0f;
        ImGui::Indent(24.0f);
        renderForm(store, uiState, formW);
        ImGui::Unindent(24.0f);

        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        if (!uiState.lastValidationOk && !uiState.lastValidationMessage.empty()) {
            ImGui::SetCursorPosX(24.0f);
            ImGui::TextColored(HEX(0xDC2626), "%s", uiState.lastValidationMessage.c_str());
        } else {
            ImGui::Dummy(ImVec2(1.0f, 18.0f));
        }

        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::SetCursorPosX(width - 250.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        if (ImGui::Button("Cancel", ImVec2(100.0f, 38.0f))) {
            uiState.lastValidationOk = true;
            uiState.lastValidationMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);

        ImGui::SameLine(0, 10.0f);
        if (gradientButton("##saveTaskModal",
                           uiState.editingTaskId > 0 ? "Save Changes" : "Create Task",
                           ImVec2(116.0f, 38.0f), 14.0f)) {
            data::Task task{};
            if (uiState.editingTaskId > 0) {
                for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                    if (store.tasks[i].id == uiState.editingTaskId) {
                        task = store.tasks[i];
                        break;
                    }
                }
            } else {
                task.id = 0;
                task.createdAt = logic::zeroDate();
                task.updatedAt = logic::zeroDate();
            }

            applyEditBuffersToTask(uiState.edit, task);
            logic::ValidationResult vr;
            if (uiState.editingTaskId > 0) {
                task.id = uiState.editingTaskId;
                if (logic::editTask(store, task, vr)) {
                    pushToast(uiState, "Task updated.");
                    uiState.lastValidationOk = true;
                    uiState.lastValidationMessage.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    uiState.lastValidationOk = vr.ok;
                    uiState.lastValidationMessage = vr.message;
                }
            } else {
                int newId = logic::createTask(store, task, vr);
                if (newId > 0) {
                    uiState.selectedTaskId = newId;
                    pushToast(uiState, "Task created.");
                    uiState.lastValidationOk = true;
                    uiState.lastValidationMessage.clear();
                    ImGui::CloseCurrentPopup();
                } else {
                    uiState.lastValidationOk = vr.ok;
                    uiState.lastValidationMessage = vr.message;
                }
            }
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::EndPopup();
        endModalSetup();
    }

    void renderConfirmDeleteDialog(data::TaskStore& store, UiState& uiState) {
        if (uiState.showConfirmDelete) {
            ImGui::OpenPopup(POPUP_CONFIRM_DEL);
            uiState.showConfirmDelete = false;
        }
        setupModal(ImVec2(440.0f, 0.0f));

        if (!ImGui::BeginPopupModal(POPUP_CONFIRM_DEL, nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            endModalSetup();
            return;
        }

        int id = uiState.pendingDeleteId;
        const data::Task* task = nullptr;
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == id) {
                task = &store.tasks[i];
                break;
            }
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Delete task");
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "This action removes the selected task and all of its descendants.");
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        if (task == nullptr) {
            ImGui::SetCursorPosX(24.0f);
            ImGui::TextColored(ColTextFaint, "Task no longer exists.");
            uiState.pendingDeleteId = -1;
        } else {
            ImGui::SetCursorPosX(24.0f);
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "#%d %s", task->id, task->title.c_str());
            ImGui::PopFont();
            ImGui::SetCursorPosX(24.0f);
            int descendants = logic::countDescendants(store, id);
            ImGui::TextColored(descendants > 0 ? HEX(0xDC2626) : ColTextFaint,
                               descendants > 0
                                   ? "Also deletes %d subtasks."
                                   : "No subtasks will be affected.",
                               descendants);
        }

        ImGui::Dummy(ImVec2(1.0f, 16.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        if (ImGui::Button("Cancel", ImVec2(100.0f, 38.0f))) {
            uiState.pendingDeleteId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);
        ImGui::SameLine(0, 10.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, HEX(0xFEF2F2));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HEX(0xFEE2E2));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, HEX(0xFECACA));
        ImGui::PushStyleColor(ImGuiCol_Text, HEX(0xB91C1C));
        if (ImGui::Button("Delete", ImVec2(100.0f, 38.0f)) && task != nullptr) {
            logic::takeSnapshot(store, uiState.undoSnapshot);
            uiState.hasUndoSnapshot = true;
            int removed = logic::deleteTaskCascade(store, id);
            std::string msg = "Deleted " + std::to_string(removed) + (removed == 1 ? " task." : " tasks.");
            pushToast(uiState, msg);
            if (uiState.selectedTaskId == id) {
                uiState.selectedTaskId = -1;
            }
            uiState.pendingDeleteId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::EndPopup();
        endModalSetup();
    }

    void renderDirtyExitConfirmDialog(data::TaskStore& store, UiState& uiState) {
        if (uiState.showDirtyExitConfirm) {
            ImGui::OpenPopup(POPUP_DIRTY_EXIT);
            uiState.showDirtyExitConfirm = false;
        }
        setupModal(ImVec2(460.0f, 0.0f));

        if (!ImGui::BeginPopupModal(POPUP_DIRTY_EXIT, nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            endModalSetup();
            return;
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Unsaved changes");
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "Save the current store before closing the application?");
        ImGui::Dummy(ImVec2(1.0f, 16.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 16.0f));

        ImGui::SetCursorPosX(24.0f);
        if (gradientButton("##saveQuit", "Save and quit", ImVec2(128.0f, 38.0f), 14.0f)) {
            uiState.triggeredSave = true;
            uiState.requestedQuit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, HEX(0xFEF2F2));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HEX(0xFEE2E2));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, HEX(0xFECACA));
        ImGui::PushStyleColor(ImGuiCol_Text, HEX(0xB91C1C));
        if (ImGui::Button("Quit without saving", ImVec2(156.0f, 38.0f))) {
            store.dirty = false;
            uiState.requestedQuit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);
        ImGui::SameLine(0, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        if (ImGui::Button("Cancel", ImVec2(88.0f, 38.0f))) {
            uiState.requestedQuit = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::EndPopup();
        endModalSetup();
    }

    void renderAboutPopup(UiState& uiState) {
        if (uiState.showAboutPopup) {
            ImGui::OpenPopup(POPUP_ABOUT);
            uiState.showAboutPopup = false;
        }
        setupModal(ImVec2(420.0f, 0.0f));

        if (!ImGui::BeginPopupModal(POPUP_ABOUT, nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
            endModalSetup();
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 min = ImGui::GetWindowPos();
        drawGradientRect(dl, min, ImVec2(min.x + ImGui::GetWindowWidth(), min.y + 6.0f), 22.0f);

        ImGui::Dummy(ImVec2(1.0f, 20.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "DataForge");
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "Hierarchical task manager built with Dear ImGui, Win32, and DirectX 11.");
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextMuted, "Key traits");
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontUiSemibold());
        ImGui::TextColored(ColTextPrimary, "Task trees, analytics, algorithm benchmarking, and keyboard-first editing.");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "This redesign consolidates the workspace into real pages instead of modal-heavy navigation.");
        ImGui::Dummy(ImVec2(1.0f, 16.0f));

        ImGui::SetCursorPosX(24.0f);
        if (gradientButton("##closeAbout", "Close", ImVec2(120.0f, 38.0f), 14.0f)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::EndPopup();
        endModalSetup();
    }

}
