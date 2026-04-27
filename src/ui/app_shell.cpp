#include "ui/app_shell.h"
#include "ui/menu_bar.h"
#include "ui/task_tree.h"
#include "ui/task_table.h"
#include "ui/task_details.h"
#include "ui/dialogs.h"
#include "ui/stats_panel.h"
#include "ui/benchmark_panel.h"
#include "ui/status_bar.h"
#include "ui/toast.h"
#include "ui/shortcuts.h"
#include "imgui.h"

namespace ui {

    static void renderMainShell(data::TaskStore& store, UiState& uiState) {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        float menuHeight   = ImGui::GetFrameHeight();
        float statusHeight = ImGui::GetFrameHeight() + 8.0f;
        ImVec2 pos  = ImVec2(vp->WorkPos.x,
                             vp->WorkPos.y + menuHeight);
        ImVec2 size = ImVec2(vp->WorkSize.x,
                             vp->WorkSize.y - menuHeight - statusHeight);
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;
        if (!ImGui::Begin("##DataForgeShell", nullptr, flags)) {
            ImGui::End();
            return;
        }
        float avail = ImGui::GetContentRegionAvail().x;
        float treeW = avail * 0.22f;
        float tableW = avail * 0.50f;
        float detailsW = avail - treeW - tableW - 16.0f;
        if (treeW    < 180.0f) treeW    = 180.0f;
        if (tableW   < 300.0f) tableW   = 300.0f;
        if (detailsW < 220.0f) detailsW = 220.0f;

        ImGui::BeginChild("##TreePane",
                          ImVec2(treeW, 0.0f), true);
        renderTaskTreePanel(store, uiState);
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("Tasks",
                          ImVec2(tableW, 0.0f), true);
        renderTaskTablePanel(store, uiState);
        ImGui::EndChild();

        ImGui::SameLine();
        ImGui::BeginChild("##DetailsPane",
                          ImVec2(0.0f, 0.0f), true);
        renderTaskDetailsPanel(store, uiState);
        ImGui::EndChild();

        ImGui::End();
    }

    void renderApp(data::TaskStore& store, UiState& uiState) {
        processKeyboardShortcuts(store, uiState);
        renderMenuBar(store, uiState);
        renderMainShell(store, uiState);
        renderStatusBar(store, uiState);

        renderAddEditDialog(store, uiState);
        renderConfirmDeleteDialog(store, uiState);
        renderDirtyExitConfirmDialog(store, uiState);
        renderAboutPopup(uiState);

        renderStatsPanel(store, uiState);
        renderBenchmarkPanel(uiState);

        renderToast(uiState);
    }

}
