#include "ui/history_panel.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "logic/history.h"
#include "data/store.h"
#include "imgui.h"

namespace ui {

    namespace {

        ImU32 colorForAction(data::HistoryAction a) {
            switch (a) {
                case data::HIST_CREATE:        return IM_COL32( 37,  99, 235, 255);
                case data::HIST_EDIT:          return IM_COL32(124,  58, 237, 255);
                case data::HIST_DELETE:        return IM_COL32(220,  38,  38, 255);
                case data::HIST_STATUS_CHANGE: return IM_COL32(217, 119,   6, 255);
                case data::HIST_LOAD:          return IM_COL32(  5, 150, 105, 255);
                case data::HIST_SAVE:          return IM_COL32( 14, 165, 233, 255);
                case data::HIST_DECISION:      return IM_COL32(236,  72, 153, 255);
            }
            return IM_COL32(100, 116, 139, 255);
        }

    }

    void renderHistoryPanel(data::TaskStore& store, UiState& uiState) {
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Action / Changes History");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted,
                           "Every mutation against the store, newest first.");
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        const char* FILTER_LABELS[] = {
            "All actions", "Create", "Edit", "Delete",
            "Status change", "Load", "Save", "Decision"
        };
        int filterIdx = uiState.historyFilter + 1;
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::Combo("##historyFilter", &filterIdx,
                         FILTER_LABELS, IM_ARRAYSIZE(FILTER_LABELS))) {
            uiState.historyFilter = filterIdx - 1;
        }
        ImGui::SameLine(0, 12.0f);
        if (ImGui::Button("Clear history", ImVec2(140.0f, 28.0f))) {
            logic::clearHistory(store);
            pushToast(uiState, "History cleared.");
        }

        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));

        std::vector<const data::HistoryEntry*> entries =
            logic::recentHistory(store, 0);

        if (entries.empty()) {
            ImGui::TextColored(ColTextFaint,
                               "No history yet. Edits and saves will appear here.");
            return;
        }

        ImGui::BeginChild("##HistoryList", ImVec2(0, 0), false);

        int shown = 0;
        for (const data::HistoryEntry* e : entries) {
            if (uiState.historyFilter >= 0 &&
                static_cast<int>(e->action) != uiState.historyFilter) {
                continue;
            }
            ++shown;

            ImVec2 min = ImGui::GetCursorScreenPos();
            float width = ImGui::GetContentRegionAvail().x;
            ImVec2 max = ImVec2(min.x + width, min.y + 64.0f);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(min, max, cardBgU32(), 14.0f);
            dl->AddRect(min, max, cardBorderU32(), 14.0f);

            ImU32 accent = colorForAction(e->action);
            dl->AddRectFilled(ImVec2(min.x + 12.0f, min.y + 12.0f),
                              ImVec2(min.x + 16.0f, max.y - 12.0f),
                              accent, 99.0f);

            ImGui::SetCursorScreenPos(ImVec2(min.x + 28.0f, min.y + 10.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(accent),
                               "%s", logic::historyActionLabel(e->action));
            ImGui::PopFont();

            ImGui::SameLine(0, 14.0f);
            ImGui::TextColored(ColTextFaint, "%s", e->timestamp.c_str());

            if (e->taskId > 0) {
                ImGui::SameLine(0, 14.0f);
                ImGui::TextColored(ColTextMuted, "task #%d", e->taskId);
            }

            ImGui::SetCursorScreenPos(ImVec2(min.x + 28.0f, min.y + 36.0f));
            ImGui::PushTextWrapPos(max.x - 12.0f);
            ImGui::TextColored(ColTextSecondary, "%s", e->summary.c_str());
            ImGui::PopTextWrapPos();

            ImGui::SetCursorScreenPos(min);
            ImGui::Dummy(ImVec2(width, 70.0f));
        }

        if (shown == 0) {
            ImGui::TextColored(ColTextFaint,
                               "No entries match the current filter.");
        }

        ImGui::EndChild();
    }

}
