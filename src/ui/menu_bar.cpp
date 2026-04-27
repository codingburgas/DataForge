#include "ui/menu_bar.h"
#include "ui/dialogs.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "logic/tasks.h"
#include "imgui.h"

namespace ui {

    void renderMenuBar(data::TaskStore& store, UiState& uiState) {
        if (!ImGui::BeginMainMenuBar()) {
            return;
        }

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                uiState.triggeredNew = true;
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                uiState.triggeredOpen = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !store.filePath.empty())) {
                uiState.triggeredSave = true;
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                uiState.triggeredSaveAs = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                uiState.requestedQuit = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Task", "Ctrl+Enter")) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            bool canEdit = uiState.selectedTaskId > 0;
            if (ImGui::MenuItem("Edit Selected...", nullptr, false, canEdit)) {
                const data::Task* t = nullptr;
                for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                    if (store.tasks[i].id == uiState.selectedTaskId) {
                        t = &store.tasks[i];
                        break;
                    }
                }
                if (t != nullptr) {
                    openEditDialog(uiState, *t);
                }
            }
            if (ImGui::MenuItem("Delete Selected", "Del", false, canEdit)) {
                openConfirmDelete(uiState, uiState.selectedTaskId);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Undo", "Ctrl+Z",
                                false, uiState.hasUndoSnapshot)) {
                if (uiState.hasUndoSnapshot) {
                    data::TaskStore copy = uiState.undoSnapshot;
                    std::string keepPath = store.filePath;
                    store          = copy;
                    store.filePath = keepPath;
                    store.dirty    = true;
                    uiState.hasUndoSnapshot = false;
                    pushToast(uiState, "Undid last destructive action.");
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Statistics",    nullptr, &uiState.showStatsPanel);
            ImGui::MenuItem("Benchmark",     nullptr, &uiState.showBenchmarkPanel);
            ImGui::Separator();
            if (ImGui::MenuItem("Dark Theme", nullptr, uiState.useDarkTheme)) {
                uiState.useDarkTheme = true;
                applyDarkTheme();
            }
            if (ImGui::MenuItem("Light Theme", nullptr, !uiState.useDarkTheme)) {
                uiState.useDarkTheme = false;
                applyLightTheme();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithms")) {
            if (ImGui::MenuItem("Sort by Priority (Bubble)",
                                nullptr,
                                uiState.sortKey  == logic::SORT_KEY_PRIORITY &&
                                uiState.sortAlgo == logic::SORT_ALGO_BUBBLE)) {
                uiState.sortKey  = logic::SORT_KEY_PRIORITY;
                uiState.sortAlgo = logic::SORT_ALGO_BUBBLE;
            }
            if (ImGui::MenuItem("Sort by Deadline (Quick)",
                                nullptr,
                                uiState.sortKey  == logic::SORT_KEY_DEADLINE &&
                                uiState.sortAlgo == logic::SORT_ALGO_QUICK)) {
                uiState.sortKey  = logic::SORT_KEY_DEADLINE;
                uiState.sortAlgo = logic::SORT_ALGO_QUICK;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About DataForge")) {
                uiState.showAboutPopup = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

}
