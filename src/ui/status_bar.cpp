#include "ui/status_bar.h"
#include "ui/theme.h"
#include "ui/i18n.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    void renderStatusBar(const data::TaskStore& store, const UiState&) {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        float barH = 34.0f;
        ImVec2 pos = ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - barH);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, barH));

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoScrollbar;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ColBgCard);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        if (ImGui::Begin("##StatusBar", nullptr, flags)) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 wPos = ImGui::GetWindowPos();
            float width = vp->WorkSize.x;

            dl->AddLine(ImVec2(wPos.x, wPos.y),
                        ImVec2(wPos.x + width, wPos.y),
                        cardBorderU32());

            ImVec2 chipMin = ImVec2(wPos.x + 10.0f, wPos.y + 6.0f);
            ImVec2 chipMax = ImVec2(wPos.x + 92.0f, wPos.y + 28.0f);
            ImU32 chipBg = isDarkTheme()
                ? (store.dirty ? IM_COL32(63, 41, 14, 255) : IM_COL32(14, 50, 36, 255))
                : (store.dirty ? IM_COL32(255, 247, 237, 255) : IM_COL32(236, 253, 245, 255));
            ImU32 chipTx = isDarkTheme()
                ? (store.dirty ? IM_COL32(252, 187, 102, 255) : IM_COL32(110, 220, 170, 255))
                : (store.dirty ? IM_COL32(180, 83, 9, 255) : IM_COL32(6, 95, 70, 255));
            dl->AddRectFilled(chipMin, chipMax, chipBg, 11.0f);
            dl->AddCircleFilled(ImVec2(chipMin.x + 12.0f, chipMin.y + 11.0f), 3.5f, chipTx);
            ImGui::SetCursorScreenPos(ImVec2(chipMin.x + 22.0f, chipMin.y + 4.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(chipTx),
                               "%s", store.dirty ? tr(K_STATE_UNSAVED) : tr(K_STATE_SAVED));
            ImGui::PopFont();

            std::string filePath = store.filePath.empty() ? std::string(tr(K_STATUS_NO_FILE)) : store.filePath;
            ImGui::SetCursorPos(ImVec2(104.0f, 8.0f));
            ImGui::TextColored(ColTextFaint, "%s", filePath.c_str());

            int overdue = logic::countOverdueTasks(store, logic::today());
            int total = static_cast<int>(store.tasks.size());
            float right = width - 214.0f;
            if (right < 360.0f) right = 360.0f;
            ImGui::SameLine(right);
            if (overdue > 0) {
                ImGui::TextColored(HEX(0xDC2626), tr(K_STATUS_OVERDUE_FMT), overdue);
                ImGui::SameLine(0, 16.0f);
            }
            ImGui::TextColored(ColTextMuted, tr(K_STATUS_TASKS_FMT), total);
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

}
