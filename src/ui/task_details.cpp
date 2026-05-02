#include "ui/task_details.h"
#include "ui/theme.h"
#include "ui/dialogs.h"
#include "logic/recursion.h"
#include "logic/tasks.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    namespace {

        const data::Task* findTask(const data::TaskStore& store, int taskId) {
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].id == taskId) {
                    return &store.tasks[i];
                }
            }
            return nullptr;
        }

        void renderMetricTile(const char* label, const char* value, float width) {
            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 68.0f);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(min, max, IM_COL32(249, 250, 252, 255), 16.0f);
            dl->AddRect(min, max, cardBorderU32(), 16.0f);
            ImGui::SetCursorScreenPos(ImVec2(min.x + 14.0f, min.y + 14.0f));
            ImGui::TextColored(ColTextFaint, "%s", label);
            ImGui::SetCursorScreenPos(ImVec2(min.x + 14.0f, min.y + 34.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "%s", value);
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(min.x, min.y));
            ImGui::Dummy(ImVec2(width, 68.0f));
        }

    }

    void renderTaskDetailsPanel(data::TaskStore& store, UiState& uiState) {
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Task Details");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted, "Selection summary, subtree metrics, and task actions.");
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 12.0f));

        if (uiState.selectedTaskId <= 0) {
            ImGui::TextColored(ColTextFaint, "Select a task to inspect it.");
            return;
        }

        const data::Task* task = findTask(store, uiState.selectedTaskId);
        if (task == nullptr) {
            uiState.selectedTaskId = -1;
            ImGui::TextColored(ColTextFaint, "Task no longer exists.");
            return;
        }

        BadgeStyle pb = priorityBadgeStyle(task->priority);
        BadgeStyle sb = statusBadgeStyle(task->status);

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 heroMin = ImGui::GetCursorScreenPos();
        ImVec2 heroMax = ImVec2(heroMin.x + ImGui::GetContentRegionAvail().x, heroMin.y + 168.0f);
        drawSoftShadow(dl, heroMin, heroMax, 20.0f);
        dl->AddRectFilled(heroMin, heroMax, cardBgU32(), 20.0f);
        dl->AddRect(heroMin, heroMax, cardBorderU32(), 20.0f);

        float completion = logic::calculateWeightedCompletion(store, task->id);
        UrgencyColor pc = colorForPriority(task->priority);
        ImU32 ring = IM_COL32((int)(pc.r * 255), (int)(pc.g * 255), (int)(pc.b * 255), 255);
        renderProgressRing(ImVec2(heroMin.x + 46.0f, heroMin.y + 54.0f), 28.0f, 5.0f,
                           completion * 100.0f, ring);
        char pct[16];
        std::snprintf(pct, sizeof(pct), "%.0f%%", completion * 100.0f);
        ImVec2 pctSz = ImGui::CalcTextSize(pct);
        dl->AddText(fontUiSemibold(), fontUiSemibold()->LegacySize,
                    ImVec2(heroMin.x + 46.0f - pctSz.x * 0.5f, heroMin.y + 48.0f),
                    IM_COL32(22, 32, 51, 255), pct);

        ImGui::SetCursorScreenPos(ImVec2(heroMin.x + 90.0f, heroMin.y + 18.0f));
        ImGui::TextColored(ColTextFaint, "#%d", task->id);
        ImGui::SetCursorScreenPos(ImVec2(heroMin.x + 90.0f, heroMin.y + 40.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "%s", task->title.c_str());
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(ImVec2(heroMin.x + 90.0f, heroMin.y + 74.0f));
        renderBadge(sb.label, sb.bg, sb.text, true, sb.dot);
        ImGui::SameLine(0, 8.0f);
        renderBadge(pb.label, pb.bg, pb.text, false, {});

        std::string note = task->description.empty() ? "No description yet." : task->description;
        ImGui::SetCursorScreenPos(ImVec2(heroMin.x + 90.0f, heroMin.y + 108.0f));
        ImGui::PushTextWrapPos(heroMax.x - 18.0f);
        ImGui::TextColored(ColTextMuted, "%s", note.c_str());
        ImGui::PopTextWrapPos();

        ImGui::Dummy(ImVec2(1.0f, 182.0f));

        char deadline[32];
        std::string due = logic::formatDate(task->deadline);
        std::snprintf(deadline, sizeof(deadline), "%s", due.empty() ? "No due date" : due.c_str());
        char est[32];
        std::snprintf(est, sizeof(est), "%d min", task->estimatedMinutes);
        char actual[32];
        std::snprintf(actual, sizeof(actual), "%d min", task->actualMinutes);
        char depth[32];
        std::snprintf(depth, sizeof(depth), "%d", logic::maxSubtreeDepth(store, task->id));
        char desc[32];
        std::snprintf(desc, sizeof(desc), "%d", logic::countDescendants(store, task->id));
        char totalEst[32];
        int total = logic::calculateTotalEstimatedMinutes(store, task->id);
        std::snprintf(totalEst, sizeof(totalEst), "%dh %02dm", total / 60, total % 60);

        float tileGap = 10.0f;
        float tileW = (ImGui::GetContentRegionAvail().x - tileGap) * 0.5f;
        renderMetricTile("Deadline", deadline, tileW);
        ImGui::SameLine(0, tileGap);
        renderMetricTile("Estimated", est, tileW);
        renderMetricTile("Actual", actual, tileW);
        ImGui::SameLine(0, tileGap);
        renderMetricTile("Subtasks", desc, tileW);
        renderMetricTile("Depth", depth, tileW);
        ImGui::SameLine(0, tileGap);
        renderMetricTile("Subtree effort", totalEst, tileW);

        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        std::string created = logic::formatDate(task->createdAt);
        std::string updated = logic::formatDate(task->updatedAt);
        ImGui::TextColored(ColTextFaint, "Created %s", created.empty() ? "-" : created.c_str());
        ImGui::SameLine(0, 18.0f);
        ImGui::TextColored(ColTextFaint, "Updated %s", updated.empty() ? "-" : updated.c_str());

        ImGui::Dummy(ImVec2(1.0f, 16.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 12.0f));

        float btnW = (ImGui::GetContentRegionAvail().x - 16.0f) / 3.0f;
        if (gradientButton("##editTask", "Edit", ImVec2(btnW, 36.0f), 14.0f)) {
            openEditDialog(uiState, *task);
        }
        ImGui::SameLine(0, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        if (ImGui::Button("Add Subtask", ImVec2(btnW, 36.0f))) {
            openCreateDialog(uiState, task->id);
        }
        ImGui::SameLine(0, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, HEX(0xFEF2F2));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HEX(0xFEE2E2));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, HEX(0xFECACA));
        ImGui::PushStyleColor(ImGuiCol_Text, HEX(0xB91C1C));
        if (ImGui::Button("Delete", ImVec2(btnW, 36.0f))) {
            openConfirmDelete(uiState, task->id);
        }
        ImGui::PopStyleColor(8);
    }

}
