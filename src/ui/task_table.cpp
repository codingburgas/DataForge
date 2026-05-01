#include "ui/task_table.h"
#include "ui/theme.h"
#include "logic/sort.h"
#include "logic/search.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    static const char* PRIORITY_FILTER_ITEMS[] = {
        "All", "Low", "Medium", "High", "Critical"
    };
    static const char* STATUS_FILTER_ITEMS[] = {
        "All", "To-do", "In progress", "Done", "Blocked"
    };
    static const char* SORT_KEY_ITEMS[] = { "Priority", "Deadline" };
    static const char* SORT_ALGO_ITEMS[] = { "Bubble", "Quick" };

    void renderTaskTablePanel(data::TaskStore& store, UiState& uiState) {
        ImGui::Text("Tasks");
        ImGui::Separator();

        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputTextWithHint("##Search",
                                 "Search title (Ctrl+F)",
                                 uiState.searchBuffer,
                                 SEARCH_BUF_SIZE);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        int priFilter = uiState.filterPriority + 1;
        if (ImGui::Combo("##PriFilter", &priFilter,
                         PRIORITY_FILTER_ITEMS,
                         IM_ARRAYSIZE(PRIORITY_FILTER_ITEMS))) {
            uiState.filterPriority = priFilter - 1;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(140.0f);
        int stFilter = uiState.filterStatus + 1;
        if (ImGui::Combo("##StFilter", &stFilter,
                         STATUS_FILTER_ITEMS,
                         IM_ARRAYSIZE(STATUS_FILTER_ITEMS))) {
            uiState.filterStatus = stFilter - 1;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        int keyIdx = static_cast<int>(uiState.sortKey);
        if (ImGui::Combo("##SortKey", &keyIdx,
                         SORT_KEY_ITEMS, IM_ARRAYSIZE(SORT_KEY_ITEMS))) {
            uiState.sortKey = static_cast<logic::SortKey>(keyIdx);
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        int algoIdx = static_cast<int>(uiState.sortAlgo);
        if (ImGui::Combo("##SortAlgo", &algoIdx,
                         SORT_ALGO_ITEMS, IM_ARRAYSIZE(SORT_ALGO_ITEMS))) {
            uiState.sortAlgo = static_cast<logic::SortAlgorithm>(algoIdx);
        }

        std::vector<int> ids = logic::filterAndSearch(
            store,
            std::string(uiState.searchBuffer),
            uiState.filterPriority,
            uiState.filterStatus);

        std::vector<data::Task> rows;
        rows.reserve(ids.size());
        for (std::size_t i = 0; i < ids.size(); ++i) {
            for (std::size_t j = 0; j < store.tasks.size(); ++j) {
                if (store.tasks[j].id == ids[i]) {
                    rows.push_back(store.tasks[j]);
                    break;
                }
            }
        }
        logic::sortTasks(rows, uiState.sortKey, uiState.sortAlgo);

        ImGuiTableFlags flags =
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_Resizable;
        ImVec2 outer_size = ImVec2(0.0f, 0.0f);
        if (!ImGui::BeginTable("##TaskTable", 6, flags, outer_size)) {
            return;
        }
        ImGui::TableSetupColumn("ID",       ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Title",    ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Status",   ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Deadline", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Est. min", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        data::Date today = logic::today();
        for (std::size_t i = 0; i < rows.size(); ++i) {
            const data::Task& t = rows[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", t.id);

            ImGui::TableSetColumnIndex(1);
            bool sel = (uiState.selectedTaskId == t.id);
            if (ImGui::Selectable(t.title.c_str(), sel,
                                  ImGuiSelectableFlags_SpanAllColumns)) {
                uiState.selectedTaskId = t.id;
            }

            ImGui::TableSetColumnIndex(2);
            UrgencyColor pc = colorForPriority(t.priority);
            ImGui::TextColored(ImVec4(pc.r, pc.g, pc.b, pc.a),
                               "%s", priorityLabel(t.priority));

            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(statusLabel(t.status));

            ImGui::TableSetColumnIndex(4);
            UrgencyColor dc = urgencyForDeadline(t.deadline, today, t.status);
            std::string ds = logic::formatDate(t.deadline);
            if (ds.empty()) {
                ImGui::TextDisabled("-");
            } else {
                ImGui::TextColored(ImVec4(dc.r, dc.g, dc.b, dc.a),
                                   "%s", ds.c_str());
            }

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%d", t.estimatedMinutes);
        }
        ImGui::EndTable();
    }

}
