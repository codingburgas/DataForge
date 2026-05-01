#include "ui/stats_panel.h"
#include "ui/theme.h"
#include "logic/recursion.h"
#include "logic/tasks.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    void renderStatsPanel(const data::TaskStore& store, UiState& uiState) {
        if (!uiState.showStatsPanel) {
            return;
        }
        ImGui::SetNextWindowSize(ImVec2(460.0f, 420.0f), ImGuiCond_Appearing);
        if (!ImGui::Begin("Statistics", &uiState.showStatsPanel)) {
            ImGui::End();
            return;
        }

        float byPriority[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float byStatus  [4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        int   totalTasks     = static_cast<int>(store.tasks.size());
        long long totalMinutes = 0;

        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            const data::Task& t = store.tasks[i];
            int p = static_cast<int>(t.priority);
            int s = static_cast<int>(t.status);
            if (p >= 0 && p < 4) byPriority[p] += 1.0f;
            if (s >= 0 && s < 4) byStatus  [s] += 1.0f;
            totalMinutes += t.estimatedMinutes;
        }

        ImGui::Text("Total tasks: %d", totalTasks);
        ImGui::Text("Total estimated minutes: %lld  (%lldh %02lldm)",
                    totalMinutes,
                    totalMinutes / 60,
                    totalMinutes % 60);
        int overdue = logic::countOverdueTasks(store, logic::today());
        ImGui::Text("Overdue: %d", overdue);
        ImGui::Separator();

        ImGui::Text("By priority");
        ImGui::PlotHistogram("##ByPriority",
                             byPriority, 4,
                             0, nullptr,
                             0.0f, FLT_MAX,
                             ImVec2(-1.0f, 80.0f));
        for (int i = 0; i < 4; ++i) {
            UrgencyColor c = colorForPriority(static_cast<data::Priority>(i));
            ImGui::TextColored(ImVec4(c.r, c.g, c.b, c.a),
                               "%s: %.0f",
                               priorityLabel(static_cast<data::Priority>(i)),
                               byPriority[i]);
            if (i < 3) {
                ImGui::SameLine();
            }
        }
        ImGui::Separator();

        ImGui::Text("By status");
        ImGui::PlotHistogram("##ByStatus",
                             byStatus, 4,
                             0, nullptr,
                             0.0f, FLT_MAX,
                             ImVec2(-1.0f, 80.0f));
        for (int i = 0; i < 4; ++i) {
            ImGui::Text("%s: %.0f",
                        statusLabel(static_cast<data::Status>(i)),
                        byStatus[i]);
            if (i < 3) {
                ImGui::SameLine();
            }
        }
        ImGui::Separator();

        ImGui::Text("Roots");
        std::vector<int> roots = logic::rootTaskIds(store);
        if (roots.empty()) {
            ImGui::TextDisabled("(no root tasks)");
        } else {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                int id = roots[i];
                int total = logic::calculateTotalEstimatedMinutes(store, id);
                int depth = logic::maxSubtreeDepth(store, id);
                int desc  = logic::countDescendants(store, id);
                float done = logic::calculateWeightedCompletion(store, id);
                const data::Task* t = nullptr;
                for (std::size_t j = 0; j < store.tasks.size(); ++j) {
                    if (store.tasks[j].id == id) {
                        t = &store.tasks[j];
                        break;
                    }
                }
                if (t == nullptr) {
                    continue;
                }
                ImGui::Text("#%d %s", id, t->title.c_str());
                ImGui::Text("  total: %dh %02dm  depth: %d  descendants: %d  done: %.0f%%",
                            total / 60, total % 60, depth, desc, done * 100.0f);
            }
        }
        ImGui::End();
    }

}
