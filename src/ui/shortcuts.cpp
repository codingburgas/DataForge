#include "ui/shortcuts.h"
#include "ui/dialogs.h"
#include "ui/toast.h"
#include "imgui.h"

namespace ui {

    void processKeyboardShortcuts(data::TaskStore& store, UiState& uiState) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantTextInput) {
            return;
        }
        bool ctrl = io.KeyCtrl;

        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
            uiState.triggeredNew = true;
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
            uiState.triggeredOpen = true;
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
            if (io.KeyShift) {
                uiState.triggeredSaveAs = true;
            } else {
                uiState.triggeredSave = true;
            }
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_F)) {
            ImGui::SetWindowFocus("Tasks");
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
            if (uiState.hasUndoSnapshot) {
                data::TaskStore copy = uiState.undoSnapshot;
                std::string keepPath = store.filePath;
                store = copy;
                store.filePath = keepPath;
                store.dirty    = true;
                uiState.hasUndoSnapshot = false;
                pushToast(uiState, "Undid last destructive action.");
            }
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Delete) && uiState.selectedTaskId > 0) {
            openConfirmDelete(uiState, uiState.selectedTaskId);
        }
        if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            openCreateDialog(uiState, uiState.selectedTaskId);
        }
    }

}
