#include "ui/menu_bar.h"
#include "ui/dialogs.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "ui/i18n.h"
#include "logic/tasks.h"
#include "data/store.h"
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

        if (ImGui::BeginMenu(tr(K_MENU_FILE))) {
            if (ImGui::MenuItem(tr(K_MENU_NEW), "Ctrl+N")) uiState.triggeredNew = true;
            if (ImGui::MenuItem(tr(K_MENU_OPEN), "Ctrl+O")) uiState.triggeredOpen = true;
            if (ImGui::MenuItem(tr(K_MENU_SAVE), "Ctrl+S", false, !store.filePath.empty())) uiState.triggeredSave = true;
            if (ImGui::MenuItem(tr(K_MENU_SAVE_AS), "Ctrl+Shift+S")) uiState.triggeredSaveAs = true;
            ImGui::Separator();
            if (ImGui::MenuItem(tr(K_MENU_EXIT), "Alt+F4")) uiState.requestedQuit = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(tr(K_MENU_EDIT))) {
            if (ImGui::MenuItem(tr(K_MENU_NEW_TASK), "Ctrl+Enter")) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            bool canEdit = uiState.selectedTaskId > 0;
            if (ImGui::MenuItem(tr(K_MENU_EDIT_SELECTED), nullptr, false, canEdit)) {
                if (const data::Task* sel =
                        data::findTaskInStoreConst(store, uiState.selectedTaskId)) {
                    openEditDialog(uiState, *sel);
                }
            }
            if (ImGui::MenuItem(tr(K_MENU_DELETE_SELECTED), "Del", false, canEdit)) {
                openConfirmDelete(uiState, uiState.selectedTaskId);
            }
            ImGui::Separator();
            if (ImGui::MenuItem(tr(K_MENU_UNDO), "Ctrl+Z", false, uiState.hasUndoSnapshot)) {
                if (uiState.hasUndoSnapshot) {
                    data::TaskStore copy = uiState.undoSnapshot;
                    std::string keepPath = store.filePath;
                    store = copy;
                    store.filePath = keepPath;
                    store.dirty = true;
                    uiState.hasUndoSnapshot = false;
                    pushToast(uiState, tr(K_TOAST_UNDO));
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(tr(K_MENU_VIEW))) {
            if (ImGui::MenuItem(tr(K_MENU_OVERVIEW), nullptr, uiState.activeNavItem == NAV_OVERVIEW)) {
                uiState.activeNavItem = NAV_OVERVIEW;
            }
            if (ImGui::MenuItem(tr(K_MENU_TASKS), nullptr, uiState.activeNavItem == NAV_TASKS)) {
                uiState.activeNavItem = NAV_TASKS;
            }
            if (ImGui::MenuItem(tr(K_MENU_ANALYTICS), nullptr, uiState.activeNavItem == NAV_ANALYTICS)) {
                uiState.activeNavItem = NAV_ANALYTICS;
            }
            if (ImGui::MenuItem(tr(K_MENU_BENCHMARK), nullptr, uiState.activeNavItem == NAV_BENCHMARK)) {
                uiState.activeNavItem = NAV_BENCHMARK;
            }
            ImGui::Separator();
            if (ImGui::MenuItem(tr(K_MENU_LIGHT_THEME), nullptr, !uiState.useDarkTheme)) {
                uiState.useDarkTheme = false;
                applyLightTheme();
            }
            if (ImGui::MenuItem(tr(K_MENU_DARK_THEME), nullptr, uiState.useDarkTheme)) {
                uiState.useDarkTheme = true;
                applyDarkTheme();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu(tr(K_MENU_LANGUAGE))) {
                if (ImGui::MenuItem(tr(K_MENU_LANG_EN), nullptr, uiState.language == LANG_EN)) {
                    uiState.language = LANG_EN;
                    setLanguage(LANG_EN);
                }
                if (ImGui::MenuItem(tr(K_MENU_LANG_ES), nullptr, uiState.language == LANG_ES)) {
                    uiState.language = LANG_ES;
                    setLanguage(LANG_ES);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(tr(K_MENU_ALGORITHMS))) {
            if (ImGui::MenuItem(tr(K_MENU_PRIORITY_ORDER), nullptr,
                    uiState.sortKey == logic::SORT_KEY_PRIORITY)) {
                uiState.sortKey = logic::SORT_KEY_PRIORITY;
            }
            if (ImGui::MenuItem(tr(K_MENU_DEADLINE_ORDER), nullptr,
                    uiState.sortKey == logic::SORT_KEY_DEADLINE)) {
                uiState.sortKey = logic::SORT_KEY_DEADLINE;
            }
            ImGui::Separator();
            if (ImGui::MenuItem(tr(K_MENU_QUICK_SORT), nullptr,
                    uiState.sortAlgo == logic::SORT_ALGO_QUICK)) {
                uiState.sortAlgo = logic::SORT_ALGO_QUICK;
            }
            if (ImGui::MenuItem(tr(K_MENU_BUBBLE_SORT), nullptr,
                    uiState.sortAlgo == logic::SORT_ALGO_BUBBLE)) {
                uiState.sortAlgo = logic::SORT_ALGO_BUBBLE;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(tr(K_MENU_HELP))) {
            if (ImGui::MenuItem(tr(K_MENU_USER_GUIDE), nullptr, uiState.activeNavItem == NAV_HELP)) {
                uiState.activeNavItem = NAV_HELP;
            }
            ImGui::Separator();
            if (ImGui::MenuItem(tr(K_MENU_ABOUT))) uiState.showAboutPopup = true;
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(5);
    }

}
