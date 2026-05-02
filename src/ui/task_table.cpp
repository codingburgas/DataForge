#include "ui/task_table.h"
#include "ui/theme.h"
#include "ui/voice_input.h"
#include "logic/sort.h"
#include "logic/search.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    namespace {

        static const char* PRIORITY_FILTER_ITEMS[] = {
            "All priorities", "Low", "Medium", "High", "Critical"
        };
        static const char* STATUS_FILTER_ITEMS[] = {
            "All statuses", "To Do", "In Progress", "Done", "Blocked"
        };
        static const char* SORT_KEY_ITEMS[]  = { "Priority", "Deadline" };
        static const char* SORT_ALGO_ITEMS[] = { "Quick", "Bubble" };

        const data::Task* findTask(const data::TaskStore& store, int taskId) {
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].id == taskId) {
                    return &store.tasks[i];
                }
            }
            return nullptr;
        }

        void renderToolbar(UiState& uiState, int count) {
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Task Queue");
            ImGui::PopFont();

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 124.0f);
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextFaint, "%d results", count);
            ImGui::PopFont();

            ImGui::Dummy(ImVec2(1.0f, 10.0f));
            ImGui::TextColored(ColTextMuted, "Search, filter, and rank work without leaving the main view.");
            ImGui::Dummy(ImVec2(1.0f, 10.0f));

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ColBgSubtle);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, HEX(0xF4F7FB));
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);

            const float micW = 48.0f;
            const float micGap = 6.0f;
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - micW - micGap);
            ImGui::InputTextWithHint("##Search", "Search tasks by title...", uiState.searchBuffer, SEARCH_BUF_SIZE);
            ImGui::SameLine(0, micGap);
            voiceMicButton("micSearch", uiState.searchBuffer, SEARCH_BUF_SIZE);

            ImGui::Dummy(ImVec2(1.0f, 10.0f));

            float avail = ImGui::GetContentRegionAvail().x;
            float w = (avail - 12.0f) * 0.5f;

            ImGui::SetNextItemWidth(w);
            int priFilter = uiState.filterPriority + 1;
            if (ImGui::Combo("##PriFilter", &priFilter,
                             PRIORITY_FILTER_ITEMS,
                             IM_ARRAYSIZE(PRIORITY_FILTER_ITEMS))) {
                uiState.filterPriority = priFilter - 1;
            }

            ImGui::SameLine(0, 12.0f);
            ImGui::SetNextItemWidth(w);
            int stFilter = uiState.filterStatus + 1;
            if (ImGui::Combo("##StFilter", &stFilter,
                             STATUS_FILTER_ITEMS,
                             IM_ARRAYSIZE(STATUS_FILTER_ITEMS))) {
                uiState.filterStatus = stFilter - 1;
            }

            ImGui::Dummy(ImVec2(1.0f, 10.0f));

            ImGui::SetNextItemWidth(w);
            int keyIdx = static_cast<int>(uiState.sortKey);
            if (ImGui::Combo("##SortKey", &keyIdx,
                             SORT_KEY_ITEMS, IM_ARRAYSIZE(SORT_KEY_ITEMS))) {
                uiState.sortKey = static_cast<logic::SortKey>(keyIdx);
            }

            ImGui::SameLine(0, 12.0f);
            ImGui::SetNextItemWidth(w);
            int algoIdx = uiState.sortAlgo == logic::SORT_ALGO_QUICK ? 0 : 1;
            if (ImGui::Combo("##SortAlgo", &algoIdx,
                             SORT_ALGO_ITEMS, IM_ARRAYSIZE(SORT_ALGO_ITEMS))) {
                uiState.sortAlgo = algoIdx == 0 ? logic::SORT_ALGO_QUICK
                                                : logic::SORT_ALGO_BUBBLE;
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar();
            ImGui::Dummy(ImVec2(1.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));
        }

        void renderTaskCard(const data::TaskStore& store, const data::Task& task,
                            bool selected, data::Date today, UiState& uiState) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            float width = ImGui::GetContentRegionAvail().x;
            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 132.0f);

            UrgencyColor pc = colorForPriority(task.priority);
            ImU32 accent = IM_COL32((int)(pc.r * 255), (int)(pc.g * 255),
                                    (int)(pc.b * 255), 255);

            drawSoftShadow(dl, min, max, 18.0f,
                           selected ? IM_COL32(124, 58, 237, 28)
                                    : IM_COL32(15, 23, 42, 12));
            dl->AddRectFilled(min, max, cardBgU32(), 18.0f);
            dl->AddRect(min, max,
                        selected ? IM_COL32(124, 58, 237, 255)
                                 : cardBorderU32(),
                        18.0f, 0, selected ? 1.8f : 1.2f);
            dl->AddRectFilled(ImVec2(min.x + 18.0f, min.y + 20.0f),
                              ImVec2(min.x + 22.0f, max.y - 20.0f),
                              accent, 99.0f);

            ImGui::SetCursorScreenPos(ImVec2(min.x + 36.0f, min.y + 18.0f));
            ImGui::PushFont(fontHeading());
            if (task.status == data::STATUS_DONE) {
                ImGui::TextColored(ColTextFaint, "%s", task.title.c_str());
            } else {
                ImGui::TextColored(ColTextPrimary, "%s", task.title.c_str());
            }
            ImGui::PopFont();

            std::string note = task.description.empty() ? "No description provided." : task.description;
            if (note.size() > 92) {
                note = note.substr(0, 92) + "...";
            }
            ImGui::SetCursorScreenPos(ImVec2(min.x + 36.0f, min.y + 48.0f));
            ImGui::TextColored(ColTextFaint, "%s", note.c_str());

            ImGui::SetCursorScreenPos(ImVec2(min.x + 36.0f, min.y + 76.0f));
            BadgeStyle sb = statusBadgeStyle(task.status);
            renderBadge(sb.label, sb.bg, sb.text, true, sb.dot);
            ImGui::SameLine(0, 8.0f);
            BadgeStyle pb = priorityBadgeStyle(task.priority);
            renderBadge(pb.label, pb.bg, pb.text, false, {});

            int descendants = logic::countDescendants(store, task.id);
            char effort[32];
            std::snprintf(effort, sizeof(effort), "%d min", task.estimatedMinutes);
            ImGui::SetCursorScreenPos(ImVec2(min.x + 36.0f, max.y - 22.0f));
            ImGui::TextColored(ColTextMuted, "%s  |  %d subtasks", effort, descendants);

            std::string due = logic::formatDate(task.deadline);
            UrgencyColor dueCol = urgencyForDeadline(task.deadline, today, task.status);
            ImVec4 dueFloat = ImVec4(dueCol.r, dueCol.g, dueCol.b, dueCol.a);
            ImGui::SetCursorScreenPos(ImVec2(max.x - 102.0f, min.y + 22.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(due.empty() ? ColTextFaint : dueFloat,
                               "%s", due.empty() ? "No due date" : due.c_str());
            ImGui::PopFont();

            char actual[32];
            std::snprintf(actual, sizeof(actual), "Actual %d min", task.actualMinutes);
            ImGui::SetCursorScreenPos(ImVec2(max.x - 104.0f, min.y + 82.0f));
            ImGui::TextColored(ColTextFaint, "%s", actual);

            ImGui::SetCursorScreenPos(min);
            char id[32];
            std::snprintf(id, sizeof(id), "##taskCard%d", task.id);
            if (ImGui::InvisibleButton(id, ImVec2(width, 132.0f))) {
                uiState.selectedTaskId = task.id;
            }

            ImGui::Dummy(ImVec2(1.0f, 10.0f));
        }

    }

    void renderTaskTablePanel(data::TaskStore& store, UiState& uiState) {
        std::vector<int> ids = logic::filterAndSearch(
            store,
            std::string(uiState.searchBuffer),
            uiState.filterPriority,
            uiState.filterStatus);

        std::vector<data::Task> rows;
        rows.reserve(ids.size());
        for (std::size_t i = 0; i < ids.size(); ++i) {
            if (const data::Task* task = findTask(store, ids[i])) {
                rows.push_back(*task);
            }
        }
        logic::sortTasks(rows, uiState.sortKey, uiState.sortAlgo);

        renderToolbar(uiState, static_cast<int>(rows.size()));
        data::Date today = logic::today();

        ImGui::BeginChild("##TaskCardList", ImVec2(0, 0), false);
        if (rows.empty()) {
            ImGui::Dummy(ImVec2(1.0f, 24.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Nothing matches the current view");
            ImGui::PopFont();
            ImGui::TextColored(ColTextFaint, "Try clearing filters or add a new task from the top bar.");
        } else {
            for (std::size_t i = 0; i < rows.size(); ++i) {
                renderTaskCard(store, rows[i], uiState.selectedTaskId == rows[i].id, today, uiState);
            }
        }
        ImGui::EndChild();
    }

}
