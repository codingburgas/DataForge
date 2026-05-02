#include "ui/dialogs.h"
#include "ui/theme.h"
#include "ui/i18n.h"
#include "ui/toast.h"
#include "ui/voice_input.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "data/store.h"
#include "imgui.h"

namespace ui {

    namespace {


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
            for (const data::Task& t : store.tasks) {
                int id = t.id;
                if (uiState.editingTaskId > 0 && id == uiState.editingTaskId) {
                    continue;
                }
                candidates.push_back(id);
            }

            std::string preview = tr(K_DLG_PARENT_ROOT);
            if (uiState.edit.parentId != -1) {
                preview = tr(K_DLG_PARENT_INVALID);
                if (const data::Task* p =
                        data::findTaskInStoreConst(store, uiState.edit.parentId)) {
                    preview = "#" + std::to_string(p->id) + " " + p->title;
                }
            }

            if (ImGui::BeginCombo("##Parent", preview.c_str())) {
                for (int cid : candidates) {
                    std::string label = tr(K_DLG_PARENT_ROOT);
                    if (cid != -1) {
                        if (const data::Task* t =
                                data::findTaskInStoreConst(store, cid)) {
                            label = "#" + std::to_string(cid) + " " + t->title;
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

            const char* PRIORITY_ITEMS[] = { tr(K_PRI_LOW), tr(K_PRI_MEDIUM), tr(K_PRI_HIGH), tr(K_PRI_CRITICAL) };
            const char* STATUS_ITEMS[]   = { tr(K_ST_TODO), tr(K_ST_IN_PROGRESS), tr(K_ST_DONE), tr(K_ST_BLOCKED) };

            fieldLabel(tr(K_DLG_TITLE_LBL));
            ImGui::SetNextItemWidth(formW - micW - micGap);
            ImGui::InputText("##Title", uiState.edit.titleBuf, TITLE_BUF_SIZE);
            ImGui::SameLine(0, micGap);
            voiceMicButton("micTitle", uiState.edit.titleBuf, TITLE_BUF_SIZE);

            fieldLabel(tr(K_DLG_DESC_LBL));
            ImGui::SetNextItemWidth(formW - micW - micGap);
            ImGui::InputTextMultiline("##Desc", uiState.edit.descBuf, DESC_BUF_SIZE,
                                      ImVec2(formW - micW - micGap, 110.0f));
            ImGui::SameLine(0, micGap);
            voiceMicButton("micDesc", uiState.edit.descBuf, DESC_BUF_SIZE, 110.0f);

            fieldLabel(tr(K_DLG_PRIORITY_LBL));
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::Combo("##Priority", &uiState.edit.priority,
                         PRIORITY_ITEMS, IM_ARRAYSIZE(PRIORITY_ITEMS));
            ImGui::SameLine(0, 12.0f);
            fieldLabel(tr(K_DLG_STATUS_LBL));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX());
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::Combo("##Status", &uiState.edit.status,
                         STATUS_ITEMS, IM_ARRAYSIZE(STATUS_ITEMS));

            fieldLabel(tr(K_DLG_DEADLINE_LBL));
            ImGui::SetNextItemWidth(formW);
            ImGui::InputText("##Deadline", uiState.edit.deadlineBuf, DATE_BUF_SIZE);

            fieldLabel(tr(K_DLG_EST_MINS_LBL));
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::InputText("##EstMins", uiState.edit.estimateBuf, NUMERIC_BUF_SIZE,
                             ImGuiInputTextFlags_CharsDecimal);
            ImGui::SameLine(0, 12.0f);
            fieldLabel(tr(K_DLG_ACT_MINS_LBL));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX());
            ImGui::SetNextItemWidth(formW * 0.48f);
            ImGui::InputText("##ActMins", uiState.edit.actualBuf, NUMERIC_BUF_SIZE,
                             ImGuiInputTextFlags_CharsDecimal);

            fieldLabel(tr(K_DLG_PARENT_LBL));
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
                           "%s", uiState.editingTaskId > 0 ? tr(K_DLG_EDIT_TASK_HEADING) : tr(K_DLG_CREATE_TASK_HEADING));
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "%s", tr(K_DLG_CAPTURE_HINT));
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
        if (ImGui::Button(tr(K_DLG_CANCEL), ImVec2(100.0f, 38.0f))) {
            uiState.lastValidationOk = true;
            uiState.lastValidationMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);

        ImGui::SameLine(0, 10.0f);
        if (gradientButton("##saveTaskModal",
                           uiState.editingTaskId > 0 ? tr(K_DLG_SAVE_CHANGES) : tr(K_DLG_CREATE_TASK_BTN),
                           ImVec2(116.0f, 38.0f), 14.0f)) {
            data::Task task{};
            if (uiState.editingTaskId > 0) {
                if (const data::Task* existing =
                        data::findTaskInStoreConst(store, uiState.editingTaskId)) {
                    task = *existing;
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
                    pushToast(uiState, tr(K_TOAST_TASK_UPDATED));
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
                    pushToast(uiState, tr(K_TOAST_TASK_CREATED));
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
        const data::Task* task = data::findTaskInStoreConst(store, id);

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "%s", tr(K_DEL_HEADING));
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "%s", tr(K_DEL_BODY));
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        if (task == nullptr) {
            ImGui::SetCursorPosX(24.0f);
            ImGui::TextColored(ColTextFaint, "%s", tr(K_DEL_TASK_GONE));
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
                                   ? tr(K_DEL_ALSO_FMT)
                                   : tr(K_DEL_NONE_AFFECTED),
                               descendants);
        }

        ImGui::Dummy(ImVec2(1.0f, 16.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        if (ImGui::Button(tr(K_DEL_CANCEL), ImVec2(100.0f, 38.0f))) {
            uiState.pendingDeleteId = -1;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);
        ImGui::SameLine(0, 10.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, HEX(0xFEF2F2));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HEX(0xFEE2E2));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, HEX(0xFECACA));
        ImGui::PushStyleColor(ImGuiCol_Text, HEX(0xB91C1C));
        if (ImGui::Button(tr(K_DEL_DELETE), ImVec2(100.0f, 38.0f)) && task != nullptr) {
            logic::takeSnapshot(store, uiState.undoSnapshot);
            uiState.hasUndoSnapshot = true;
            int removed = logic::deleteTaskCascade(store, id);
            char buf[96];
            std::snprintf(buf, sizeof(buf),
                removed == 1 ? tr(K_TOAST_DELETED_ONE_FMT) : tr(K_TOAST_DELETED_MANY_FMT),
                removed);
            pushToast(uiState, std::string(buf));
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
        ImGui::TextColored(ColTextPrimary, "%s", tr(K_QUIT_HEADING));
        ImGui::PopFont();
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "%s", tr(K_QUIT_BODY));
        ImGui::Dummy(ImVec2(1.0f, 16.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 16.0f));

        ImGui::SetCursorPosX(24.0f);
        if (gradientButton("##saveQuit", tr(K_QUIT_SAVE_AND_QUIT), ImVec2(128.0f, 38.0f), 14.0f)) {
            uiState.triggeredSave = true;
            uiState.requestedQuit = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0, 10.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, HEX(0xFEF2F2));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HEX(0xFEE2E2));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, HEX(0xFECACA));
        ImGui::PushStyleColor(ImGuiCol_Text, HEX(0xB91C1C));
        if (ImGui::Button(tr(K_QUIT_WITHOUT_SAVING), ImVec2(156.0f, 38.0f))) {
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
        if (ImGui::Button(tr(K_QUIT_CANCEL), ImVec2(88.0f, 38.0f))) {
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
        ImGui::TextColored(ColTextFaint, "%s", tr(K_ABOUT_BODY));
        ImGui::Dummy(ImVec2(1.0f, 14.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextMuted, "%s", tr(K_ABOUT_KEY_TRAITS_LBL));
        ImGui::SetCursorPosX(24.0f);
        ImGui::PushFont(fontUiSemibold());
        ImGui::TextColored(ColTextPrimary, "%s", tr(K_ABOUT_KEY_TRAITS));
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::SetCursorPosX(24.0f);
        ImGui::TextColored(ColTextFaint, "%s", tr(K_ABOUT_REDESIGN));
        ImGui::Dummy(ImVec2(1.0f, 16.0f));

        ImGui::SetCursorPosX(24.0f);
        if (gradientButton("##closeAbout", tr(K_ABOUT_CLOSE), ImVec2(120.0f, 38.0f), 14.0f)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::EndPopup();
        endModalSetup();
    }

}
