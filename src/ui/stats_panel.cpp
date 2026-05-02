#include "ui/stats_panel.h"
#include "ui/theme.h"
#include "logic/recursion.h"
#include "logic/tasks.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    namespace {

        void renderStatCard(const char* id, const char* label, const char* value,
                            const char* sub, ImU32 accent, float width) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ColBgCard);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 18.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::BeginChild(id, ImVec2(width, 104.0f), true);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetWindowPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 104.0f);
            drawSoftShadow(dl, min, max, 18.0f);
            dl->AddRectFilled(min, max, cardBgU32(), 18.0f);
            dl->AddRect(min, max, cardBorderU32(), 18.0f);
            dl->AddRectFilled(ImVec2(min.x + 16.0f, min.y + 18.0f),
                              ImVec2(min.x + 56.0f, min.y + 58.0f),
                              IM_COL32((accent >> IM_COL32_R_SHIFT) & 255,
                                       (accent >> IM_COL32_G_SHIFT) & 255,
                                       (accent >> IM_COL32_B_SHIFT) & 255, 20), 12.0f);
            dl->AddCircleFilled(ImVec2(min.x + 36.0f, min.y + 38.0f), 6.0f, accent);

            ImGui::SetCursorPos(ImVec2(74.0f, 16.0f));
            ImGui::PushFont(fontDisplay());
            ImGui::TextColored(ColTextPrimary, "%s", value);
            ImGui::PopFont();
            ImGui::SetCursorPos(ImVec2(74.0f, 56.0f));
            ImGui::TextColored(ColTextMuted, "%s", label);
            if (sub != nullptr && sub[0] != '\0') {
                ImGui::SetCursorPos(ImVec2(74.0f, 76.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(accent), "%s", sub);
                ImGui::PopFont();
            }

            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
        }

        void renderDistributionBar(const float* vals, const ImU32* cols,
                                   int count, float total, float width, float height) {
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
                              IM_COL32(241, 245, 249, 255), 99.0f);
            float x = pos.x;
            if (total > 0.0f) {
                for (int i = 0; i < count; ++i) {
                    float segW = width * (vals[i] / total);
                    if (segW < 1.0f) {
                        continue;
                    }
                    dl->AddRectFilled(ImVec2(x, pos.y),
                                      ImVec2(x + segW, pos.y + height),
                                      cols[i], 99.0f);
                    x += segW;
                }
            }
            ImGui::Dummy(ImVec2(width, height));
        }

        const data::Task* findTask(const data::TaskStore& store, int taskId) {
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].id == taskId) {
                    return &store.tasks[i];
                }
            }
            return nullptr;
        }

    }

    void renderStatsPanel(const data::TaskStore& store, UiState&) {
        float byPriority[4] = {};
        float byStatus[4] = {};
        int totalTasks = static_cast<int>(store.tasks.size());
        long long totalMins = 0;

        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            const data::Task& t = store.tasks[i];
            int p = static_cast<int>(t.priority);
            int s = static_cast<int>(t.status);
            if (p >= 0 && p < 4) byPriority[p] += 1.0f;
            if (s >= 0 && s < 4) byStatus[s] += 1.0f;
            totalMins += t.estimatedMinutes;
        }

        int overdue = logic::countOverdueTasks(store, logic::today());
        std::vector<int> roots = logic::rootTaskIds(store);

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Workspace Analytics");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted, "Measure task mix, delivery risk, and project completion in one place.");
        ImGui::Dummy(ImVec2(1.0f, 18.0f));

        float avail = ImGui::GetContentRegionAvail().x;
        float gap = 14.0f;
        float cardW = (avail - gap * 3.0f) * 0.25f;

        char totalBuf[32];
        std::snprintf(totalBuf, sizeof(totalBuf), "%d", totalTasks);
        char completedBuf[32];
        std::snprintf(completedBuf, sizeof(completedBuf), "%.0f", byStatus[data::STATUS_DONE]);
        char rootsBuf[32];
        std::snprintf(rootsBuf, sizeof(rootsBuf), "%d", static_cast<int>(roots.size()));
        char overdueBuf[32];
        std::snprintf(overdueBuf, sizeof(overdueBuf), "%d", overdue);
        char minutesBuf[32];
        std::snprintf(minutesBuf, sizeof(minutesBuf), "%lldh %02lldm", totalMins / 60, totalMins % 60);

        renderStatCard("##stTotal", "Total Tasks", totalBuf, minutesBuf,
                       IM_COL32(124, 58, 237, 255), cardW);
        ImGui::SameLine(0, gap);
        renderStatCard("##stDone", "Completed", completedBuf, "Finished work",
                       IM_COL32(5, 150, 105, 255), cardW);
        ImGui::SameLine(0, gap);
        renderStatCard("##stRoots", "Root Projects", rootsBuf, "Top-level trees",
                       IM_COL32(37, 99, 235, 255), cardW);
        ImGui::SameLine(0, gap);
        renderStatCard("##stOverdue", "Overdue", overdueBuf, overdue > 0 ? "Needs attention" : "Healthy",
                       IM_COL32(220, 38, 38, 255), cardW);

        ImGui::Dummy(ImVec2(1.0f, 18.0f));

        float sideW = 360.0f;
        if (avail < 1000.0f) {
            sideW = 320.0f;
        }
        float mainW = avail - sideW - 20.0f;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        ImGui::BeginChild("##AnalyticsMain", ImVec2(mainW, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);

        float analyticsW = ImGui::GetContentRegionAvail().x;

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Priority distribution");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));
        ImU32 priCols[4] = {
            IM_COL32(5, 150, 105, 255),
            IM_COL32(37, 99, 235, 255),
            IM_COL32(234, 88, 12, 255),
            IM_COL32(220, 38, 38, 255),
        };
        renderDistributionBar(byPriority, priCols, 4, (float)totalTasks, analyticsW, 12.0f);
        ImGui::Dummy(ImVec2(1.0f, 8.0f));
        for (int i = 0; i < 4; ++i) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(priCols[i]),
                               "%s %.0f", priorityLabel(static_cast<data::Priority>(i)), byPriority[i]);
            if (i < 3) ImGui::SameLine(0, 18.0f);
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Status distribution");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));
        ImU32 stCols[4] = {
            IM_COL32(148, 163, 184, 255),
            IM_COL32(217, 119,   6, 255),
            IM_COL32(  5, 150, 105, 255),
            IM_COL32(220,  38,  38, 255),
        };
        renderDistributionBar(byStatus, stCols, 4, (float)totalTasks, analyticsW, 12.0f);
        ImGui::Dummy(ImVec2(1.0f, 8.0f));
        for (int i = 0; i < 4; ++i) {
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(stCols[i]),
                               "%s %.0f", statusLabel(static_cast<data::Status>(i)), byStatus[i]);
            if (i < 3) ImGui::SameLine(0, 18.0f);
        }

        ImGui::Dummy(ImVec2(1.0f, 18.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Project health");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 10.0f));

        if (roots.empty()) {
            ImGui::TextColored(ColTextFaint, "No root projects yet.");
        } else {
            for (std::size_t i = 0; i < roots.size(); ++i) {
                const data::Task* root = findTask(store, roots[i]);
                if (root == nullptr) {
                    continue;
                }

                ImVec2 min = ImGui::GetCursorScreenPos();
                ImVec2 max = ImVec2(min.x + analyticsW, min.y + 84.0f);
                ImDrawList* dl = ImGui::GetWindowDrawList();
                dl->AddRectFilled(min, max, cardBgU32(), 18.0f);
                dl->AddRect(min, max, cardBorderU32(), 18.0f);

                float completion = logic::calculateWeightedCompletion(store, root->id);
                UrgencyColor pc = colorForPriority(root->priority);
                ImU32 accent = IM_COL32((int)(pc.r * 255), (int)(pc.g * 255), (int)(pc.b * 255), 255);
                renderProgressRing(ImVec2(min.x + 32.0f, min.y + 42.0f), 16.0f, 4.0f,
                                   completion * 100.0f, accent);

                ImGui::SetCursorScreenPos(ImVec2(min.x + 64.0f, min.y + 18.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%s", root->title.c_str());
                ImGui::PopFont();
                ImGui::SetCursorScreenPos(ImVec2(min.x + 64.0f, min.y + 42.0f));
                ImGui::TextColored(ColTextFaint,
                                   "%dh %02dm  |  %d subtasks  |  depth %d",
                                   logic::calculateTotalEstimatedMinutes(store, root->id) / 60,
                                   logic::calculateTotalEstimatedMinutes(store, root->id) % 60,
                                   logic::countDescendants(store, root->id),
                                   logic::maxSubtreeDepth(store, root->id));
                ImGui::SetCursorScreenPos(ImVec2(max.x - 74.0f, min.y + 30.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextMuted, "%.0f%%", completion * 100.0f);
                ImGui::PopFont();
                ImGui::Dummy(ImVec2(1.0f, 96.0f));
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::SameLine(0, 20.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        ImGui::BeginChild("##AnalyticsSide", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);

        float signalsW = ImGui::GetContentRegionAvail().x;

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Signals");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));

        ImVec2 cardMin = ImGui::GetCursorScreenPos();
        ImVec2 cardMax = ImVec2(cardMin.x + signalsW, cardMin.y + 156.0f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(cardMin, cardMax, cardBgU32(), 18.0f);
        dl->AddRect(cardMin, cardMax, cardBorderU32(), 18.0f);
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 18.0f));
        ImGui::PushFont(fontUiSemibold());
        ImGui::TextColored(ColTextPrimary, "Overdue pressure");
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 48.0f));
        ImGui::PushFont(fontDisplay());
        ImGui::TextColored(overdue > 0 ? HEX(0xDC2626) : HEX(0x059669), "%d", overdue);
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 88.0f));
        ImGui::TextColored(ColTextFaint,
                           overdue > 0 ? "Tasks are overdue and should be reviewed."
                                       : "No overdue items right now.");
        ImGui::Dummy(ImVec2(1.0f, 168.0f));

        cardMin = ImGui::GetCursorScreenPos();
        cardMax = ImVec2(cardMin.x + signalsW, cardMin.y + 156.0f);
        dl->AddRectFilled(cardMin, cardMax, cardBgU32(), 18.0f);
        dl->AddRect(cardMin, cardMax, cardBorderU32(), 18.0f);
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 18.0f));
        ImGui::PushFont(fontUiSemibold());
        ImGui::TextColored(ColTextPrimary, "Execution mix");
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 50.0f));
        ImGui::TextColored(ColTextMuted, "%.0f in progress", byStatus[data::STATUS_IN_PROGRESS]);
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 74.0f));
        ImGui::TextColored(ColTextMuted, "%.0f blocked", byStatus[data::STATUS_BLOCKED]);
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 98.0f));
        ImGui::TextColored(ColTextMuted, "%.0f done", byStatus[data::STATUS_DONE]);
        ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 122.0f));
        ImGui::TextColored(ColTextFaint, "Use this to gauge whether work is flowing or stalling.");
        ImGui::Dummy(ImVec2(1.0f, 168.0f));

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

}
