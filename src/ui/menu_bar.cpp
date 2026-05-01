#include "ui/menu_bar.h"
#include "ui/dialogs.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "logic/tasks.h"
#include "imgui.h"

namespace ui {

    void renderMenuBar(data::TaskStore& store, UiState& uiState) {
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ColBgCard);
        ImGui::PushStyleColor(ImGuiCol_Header, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ColBgCard);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 14.0f);

        if (!ImGui::BeginMainMenuBar()) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(5);
            return;
        }

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) uiState.triggeredNew = true;
            if (ImGui::MenuItem("Open...", "Ctrl+O")) uiState.triggeredOpen = true;
            if (ImGui::MenuItem("Save", "Ctrl+S", false, !store.filePath.empty())) uiState.triggeredSave = true;
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) uiState.triggeredSaveAs = true;
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) uiState.requestedQuit = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("New Task", "Ctrl+Enter")) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            bool canEdit = uiState.selectedTaskId > 0;
            if (ImGui::MenuItem("Edit Selected...", nullptr, false, canEdit)) {
                for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                    if (store.tasks[i].id == uiState.selectedTaskId) {
                        openEditDialog(uiState, store.tasks[i]);
                        break;
                    }
                }
            }
            if (ImGui::MenuItem("Delete Selected", "Del", false, canEdit)) {
                openConfirmDelete(uiState, uiState.selectedTaskId);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, uiState.hasUndoSnapshot)) {
                if (uiState.hasUndoSnapshot) {
                    data::TaskStore copy = uiState.undoSnapshot;
                    std::string keepPath = store.filePath;
                    store = copy;
                    store.filePath = keepPath;
                    store.dirty = true;
                    uiState.hasUndoSnapshot = false;
                    pushToast(uiState, "Undid last action.");
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Overview", nullptr, uiState.activeNavItem == NAV_OVERVIEW)) {
                uiState.activeNavItem = NAV_OVERVIEW;
            }
            if (ImGui::MenuItem("My Tasks", nullptr, uiState.activeNavItem == NAV_TASKS)) {
                uiState.activeNavItem = NAV_TASKS;
            }
            if (ImGui::MenuItem("Analytics", nullptr, uiState.activeNavItem == NAV_ANALYTICS)) {
                uiState.activeNavItem = NAV_ANALYTICS;
            }
            if (ImGui::MenuItem("Benchmark", nullptr, uiState.activeNavItem == NAV_BENCHMARK)) {
                uiState.activeNavItem = NAV_BENCHMARK;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Light Theme", nullptr, !uiState.useDarkTheme)) {
                uiState.useDarkTheme = false;
                applyLightTheme();
            }
            if (ImGui::MenuItem("Dark Theme", nullptr, uiState.useDarkTheme)) {
                uiState.useDarkTheme = true;
                applyDarkTheme();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Algorithms")) {
            if (ImGui::MenuItem("Priority order", nullptr,
                    uiState.sortKey == logic::SORT_KEY_PRIORITY)) {
                uiState.sortKey = logic::SORT_KEY_PRIORITY;
            }
            if (ImGui::MenuItem("Deadline order", nullptr,
                    uiState.sortKey == logic::SORT_KEY_DEADLINE)) {
                uiState.sortKey = logic::SORT_KEY_DEADLINE;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quick sort", nullptr,
                    uiState.sortAlgo == logic::SORT_ALGO_QUICK)) {
                uiState.sortAlgo = logic::SORT_ALGO_QUICK;
            }
            if (ImGui::MenuItem("Bubble sort", nullptr,
                    uiState.sortAlgo == logic::SORT_ALGO_BUBBLE)) {
                uiState.sortAlgo = logic::SORT_ALGO_BUBBLE;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About DataForge")) uiState.showAboutPopup = true;
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
    }

}
