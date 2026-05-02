#include "ui/task_tree.h"
#include "ui/theme.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "data/store.h"
#include "imgui.h"

namespace ui {

    namespace {

        void renderTreeNode(data::TaskStore& store,
                            UiState& uiState,
                            int taskId,
                            int depth) {
            if (depth > logic::MAX_TREE_DEPTH) {
                ImGui::TextColored(ColTextFaint, "(depth cap)");
                return;
            }

            const data::Task* task = data::findTaskInStoreConst(store, taskId);
            if (task == nullptr) {
                return;
            }

            std::vector<int> children = logic::childTaskIds(store, taskId);
            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth |
                ImGuiTreeNodeFlags_FramePadding;
            if (children.empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }
            if (uiState.selectedTaskId == taskId) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            ImGui::PushStyleColor(ImGuiCol_Header, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ColBgHover);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_Text,
                                  task->status == data::STATUS_DONE ? ColTextFaint : ColTextSecondary);

            bool opened = ImGui::TreeNodeEx(
                reinterpret_cast<void*>(static_cast<std::intptr_t>(taskId)),
                flags, "%s", task->title.c_str());

            ImGui::PopStyleColor(4);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImGui::GetItemRectMax();
            UrgencyColor pc = colorForPriority(task->priority);
            ImU32 accent = urgencyToImU32(pc);
            float dotX = itemMin.x + ImGui::GetTreeNodeToLabelSpacing() - 10.0f;
            float dotY = (itemMin.y + itemMax.y) * 0.5f;
            dl->AddCircleFilled(ImVec2(dotX, dotY), 3.5f, accent);

            if (uiState.selectedTaskId == taskId) {
                dl->AddRectFilled(ImVec2(itemMin.x, itemMin.y + 5.0f),
                                  ImVec2(itemMin.x + 3.0f, itemMax.y - 5.0f),
                                  GradLeft, 3.0f);
            }

            if (ImGui::IsItemClicked()) {
                uiState.selectedTaskId = taskId;
            }

            if (opened && !children.empty()) {
                for (int childId : children) {
                    renderTreeNode(store, uiState, childId, depth + 1);
                }
                ImGui::TreePop();
            }
        }

    }

    void renderTaskTreePanel(data::TaskStore& store, UiState& uiState) {
        std::vector<int> roots = logic::rootTaskIds(store);

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Project Tree");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted, "%d root items", static_cast<int>(roots.size()));
        ImGui::Dummy(ImVec2(1.0f, 10.0f));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));

        if (store.tasks.empty()) {
            ImGui::TextColored(ColTextFaint, "No tasks yet.");
            return;
        }
        if (roots.empty()) {
            ImGui::TextColored(ColTextFaint, "No root tasks.");
            return;
        }

        if (uiState.selectedTaskId > 0) {
            const data::Task* selected = data::findTaskInStoreConst(store, uiState.selectedTaskId);
            if (selected != nullptr) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 min = ImGui::GetCursorScreenPos();
                ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + 64.0f);
                dl->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 16.0f);
                dl->AddRect(min, max, cardBorderU32(), 16.0f);
                ImGui::SetCursorScreenPos(ImVec2(min.x + 16.0f, min.y + 14.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "#%d %s", selected->id, selected->title.c_str());
                ImGui::PopFont();
                ImGui::SetCursorScreenPos(ImVec2(min.x + 16.0f, min.y + 36.0f));
                ImGui::TextColored(ColTextFaint, "%d descendants  |  depth %d",
                                   logic::countDescendants(store, selected->id),
                                   logic::maxSubtreeDepth(store, selected->id));
                ImGui::Dummy(ImVec2(1.0f, 74.0f));
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 18.0f);
        for (int rootId : roots) {
            renderTreeNode(store, uiState, rootId, 0);
        }
        ImGui::PopStyleVar(2);
    }

}
