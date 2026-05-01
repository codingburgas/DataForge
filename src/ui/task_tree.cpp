#include "ui/task_tree.h"
#include "ui/theme.h"
#include "logic/tasks.h"
#include "imgui.h"

namespace ui {

    static void renderNode(data::TaskStore& store,
                           UiState& uiState,
                           int taskId,
                           int depth) {
        if (depth > logic::MAX_TREE_DEPTH) {
            ImGui::TextDisabled("(depth cap hit)");
            return;
        }
        const data::Task* t = nullptr;
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == taskId) {
                t = &store.tasks[i];
                break;
            }
        }
        if (t == nullptr) {
            return;
        }
        std::vector<int> children = logic::childTaskIds(store, taskId);
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth;
        if (children.empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf |
                     ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }
        if (uiState.selectedTaskId == taskId) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        UrgencyColor pc = colorForPriority(t->priority);
        ImGui::PushStyleColor(ImGuiCol_Text,
                              ImVec4(pc.r, pc.g, pc.b, pc.a));
        bool opened = ImGui::TreeNodeEx(
            reinterpret_cast<void*>(static_cast<std::intptr_t>(taskId)),
            flags, "%s", t->title.c_str());
        ImGui::PopStyleColor();
        if (ImGui::IsItemClicked()) {
            uiState.selectedTaskId = taskId;
        }
        if (opened && !children.empty()) {
            for (std::size_t i = 0; i < children.size(); ++i) {
                renderNode(store, uiState, children[i], depth + 1);
            }
            ImGui::TreePop();
        }
    }

    void renderTaskTreePanel(data::TaskStore& store, UiState& uiState) {
        ImGui::Text("Hierarchy");
        ImGui::Separator();
        if (store.tasks.empty()) {
            ImGui::TextDisabled("No tasks yet. Use Edit > New Task.");
            return;
        }
        std::vector<int> roots = logic::rootTaskIds(store);
        if (roots.empty()) {
            ImGui::TextDisabled("No root tasks.");
            return;
        }
        for (std::size_t i = 0; i < roots.size(); ++i) {
            renderNode(store, uiState, roots[i], 0);
        }
    }

}
