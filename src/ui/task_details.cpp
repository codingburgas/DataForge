#include "ui/task_details.h"
#include "ui/theme.h"
#include "ui/dialogs.h"
#include "logic/recursion.h"
#include "logic/tasks.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    static void renderMetric(const char* label, const char* value) {
        ImGui::Text("%s:", label);
        ImGui::SameLine();
        ImGui::TextUnformatted(value);
    }

    void renderTaskDetailsPanel(data::TaskStore& store, UiState& uiState) {
        ImGui::Text("Details");
        ImGui::Separator();
        if (uiState.selectedTaskId <= 0) {
            ImGui::TextDisabled("No task selected.");
            return;
        }
        const data::Task* t = nullptr;
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == uiState.selectedTaskId) {
                t = &store.tasks[i];
                break;
            }
        }
        if (t == nullptr) {
            uiState.selectedTaskId = -1;
            ImGui::TextDisabled("Task no longer exists.");
            return;
        }
        ImGui::Text("#%d", t->id);
        ImGui::SameLine();
        UrgencyColor pc = colorForPriority(t->priority);
        ImGui::TextColored(ImVec4(pc.r, pc.g, pc.b, pc.a),
                           "[%s]", priorityLabel(t->priority));
        ImGui::SameLine();
        ImGui::Text("[%s]", statusLabel(t->status));

        ImGui::TextWrapped("%s", t->title.c_str());
        ImGui::Spacing();
        if (!t->description.empty()) {
            ImGui::TextWrapped("%s", t->description.c_str());
            ImGui::Spacing();
        }

        std::string deadline = logic::formatDate(t->deadline);
        renderMetric("Deadline",
                     deadline.empty() ? "-" : deadline.c_str());

        char buf[64];
        std::snprintf(buf, sizeof(buf), "%d min", t->estimatedMinutes);
        renderMetric("Estimated", buf);
        std::snprintf(buf, sizeof(buf), "%d min", t->actualMinutes);
        renderMetric("Actual", buf);

        std::string created = logic::formatDate(t->createdAt);
        std::string updated = logic::formatDate(t->updatedAt);
        renderMetric("Created", created.empty() ? "-" : created.c_str());
        renderMetric("Updated", updated.empty() ? "-" : updated.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Subtree");

        int total = logic::calculateTotalEstimatedMinutes(store, t->id);
        int hours = total / 60;
        int mins  = total % 60;
        std::snprintf(buf, sizeof(buf), "%dh %02dm", hours, mins);
        renderMetric("Total est.", buf);

        float completion = logic::calculateWeightedCompletion(store, t->id);
        std::snprintf(buf, sizeof(buf), "%.0f%%", completion * 100.0f);
        renderMetric("Weighted done", buf);

        int descendants = logic::countDescendants(store, t->id);
        std::snprintf(buf, sizeof(buf), "%d", descendants);
        renderMetric("Descendants", buf);

        int depth = logic::maxSubtreeDepth(store, t->id);
        std::snprintf(buf, sizeof(buf), "%d", depth);
        renderMetric("Depth", buf);

        ImGui::ProgressBar(completion, ImVec2(-FLT_MIN, 0.0f));

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("Edit")) {
            openEditDialog(uiState, *t);
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Subtask")) {
            openCreateDialog(uiState, t->id);
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete")) {
            openConfirmDelete(uiState, t->id);
        }
    }

}
