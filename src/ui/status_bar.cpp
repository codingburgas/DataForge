#include "ui/status_bar.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    void renderStatusBar(const data::TaskStore& store, const UiState& uiState) {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        float barHeight = ImGui::GetFrameHeight() + 8.0f;
        ImVec2 pos;
        pos.x = vp->WorkPos.x;
        pos.y = vp->WorkPos.y + vp->WorkSize.y - barHeight;
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, barHeight));
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus;
        if (ImGui::Begin("##StatusBar", nullptr, flags)) {
            int total   = static_cast<int>(store.tasks.size());
            int overdue = logic::countOverdueTasks(store, logic::today());
            const char* dirtyStr = store.dirty ? "*" : " ";
            std::string filePath = store.filePath.empty()
                ? std::string("<unsaved>")
                : store.filePath;
            ImGui::Text("%s %s | %d tasks | %d overdue | theme: %s",
                        dirtyStr,
                        filePath.c_str(),
                        total,
                        overdue,
                        uiState.useDarkTheme ? "dark" : "light");
        }
        ImGui::End();
    }

}
