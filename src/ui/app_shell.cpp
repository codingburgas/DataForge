#include "ui/app_shell.h"
#include "ui/menu_bar.h"
#include "ui/task_tree.h"
#include "ui/task_table.h"
#include "ui/task_details.h"
#include "ui/dialogs.h"
#include "ui/stats_panel.h"
#include "ui/benchmark_panel.h"
#include "ui/status_bar.h"
#include "ui/toast.h"
#include "ui/shortcuts.h"
#include "ui/theme.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "logic/sort.h"
#include "imgui.h"

namespace ui {

    namespace {

        static const float TOPBAR_H        = 76.0f;
        static const float STATUSBAR_H     = 34.0f;
        static const float SIDEBAR_W_FULL  = 244.0f;
        static const float SIDEBAR_W_SLIM  = 76.0f;

        const data::Task* findTask(const data::TaskStore& store, int taskId) {
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].id == taskId) {
                    return &store.tasks[i];
                }
            }
            return nullptr;
        }

        std::vector<data::Task> collectFocusTasks(const data::TaskStore& store, std::size_t limit) {
            std::vector<data::Task> out;
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].status != data::STATUS_DONE) {
                    out.push_back(store.tasks[i]);
                }
            }
            logic::sortTasks(out, logic::SORT_KEY_PRIORITY, logic::SORT_ALGO_QUICK);
            if (out.size() > limit) {
                out.resize(limit);
            }
            return out;
        }

        std::vector<const data::Task*> collectRecentTasks(const data::TaskStore& store, std::size_t limit) {
            std::vector<const data::Task*> out;
            out.reserve(store.tasks.size());
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                out.push_back(&store.tasks[i]);
            }
            std::sort(out.begin(), out.end(),
                [](const data::Task* a, const data::Task* b) {
                    int byUpdated = logic::compareDates(a->updatedAt, b->updatedAt);
                    if (byUpdated != 0) {
                        return byUpdated > 0;
                    }
                    return a->id > b->id;
                });
            if (out.size() > limit) {
                out.resize(limit);
            }
            return out;
        }

        const char* pageTitle(NavItem item) {
            switch (item) {
                case NAV_OVERVIEW:  return "Overview";
                case NAV_TASKS:     return "My Tasks";
                case NAV_ANALYTICS: return "Analytics";
                case NAV_BENCHMARK: return "Benchmark";
            }
            return "Workspace";
        }

        const char* pageSubtitle(NavItem item) {
            switch (item) {
                case NAV_OVERVIEW:  return "Here is what is moving across the workspace.";
                case NAV_TASKS:     return "Manage hierarchy, focus work, and edit task details.";
                case NAV_ANALYTICS: return "Track delivery health, workload mix, and project progress.";
                case NAV_BENCHMARK: return "Compare sorting strategies on synthetic task sets.";
            }
            return "";
        }

        std::string greetingTitle() {
            std::time_t now = std::time(nullptr);
            std::tm local{};
            localtime_s(&local, &now);
            if (local.tm_hour < 12) {
                return "Good morning";
            }
            if (local.tm_hour < 18) {
                return "Good afternoon";
            }
            return "Good evening";
        }

        void drawNavIcon(ImDrawList* dl, ImVec2 centre, NavItem item, ImU32 color) {
            switch (item) {
                case NAV_OVERVIEW:
                    dl->AddRect(ImVec2(centre.x - 6.0f, centre.y - 6.0f),
                                ImVec2(centre.x + 6.0f, centre.y + 6.0f),
                                color, 3.0f, 0, 1.6f);
                    dl->AddCircleFilled(ImVec2(centre.x, centre.y), 1.8f, color);
                    break;
                case NAV_TASKS:
                    dl->AddRect(ImVec2(centre.x - 6.0f, centre.y - 7.0f),
                                ImVec2(centre.x + 6.0f, centre.y + 7.0f),
                                color, 3.0f, 0, 1.5f);
                    dl->AddLine(ImVec2(centre.x - 3.0f, centre.y - 1.0f),
                                ImVec2(centre.x + 3.0f, centre.y - 1.0f), color, 1.5f);
                    dl->AddLine(ImVec2(centre.x - 3.0f, centre.y + 3.0f),
                                ImVec2(centre.x + 3.0f, centre.y + 3.0f), color, 1.5f);
                    break;
                case NAV_ANALYTICS:
                    dl->AddLine(ImVec2(centre.x - 6.0f, centre.y + 5.0f),
                                ImVec2(centre.x - 6.0f, centre.y - 1.0f), color, 2.0f);
                    dl->AddLine(ImVec2(centre.x, centre.y + 5.0f),
                                ImVec2(centre.x, centre.y - 5.0f), color, 2.0f);
                    dl->AddLine(ImVec2(centre.x + 6.0f, centre.y + 5.0f),
                                ImVec2(centre.x + 6.0f, centre.y - 3.0f), color, 2.0f);
                    break;
                case NAV_BENCHMARK:
                    dl->AddRect(ImVec2(centre.x - 6.0f, centre.y - 6.0f),
                                ImVec2(centre.x + 6.0f, centre.y + 6.0f),
                                color, 3.0f, 0, 1.4f);
                    dl->AddCircleFilled(ImVec2(centre.x - 2.0f, centre.y - 1.0f), 1.5f, color);
                    dl->AddCircleFilled(ImVec2(centre.x + 2.0f, centre.y - 1.0f), 1.5f, color);
                    dl->AddLine(ImVec2(centre.x - 4.0f, centre.y + 3.0f),
                                ImVec2(centre.x + 4.0f, centre.y + 3.0f), color, 1.5f);
                    break;
            }
        }

        void renderSidebar(const data::TaskStore& store, UiState& uiState,
                           float menuBarH, float sidebarW) {
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            float sidebarH = vp->WorkSize.y - menuBarH - STATUSBAR_H;

            ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, vp->WorkPos.y + menuBarH));
            ImGui::SetNextWindowSize(ImVec2(sidebarW, sidebarH));

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNav;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ColBgCard);
            ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            if (!ImGui::Begin("##Sidebar", nullptr, flags)) {
                ImGui::End();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor(2);
                return;
            }

            bool expanded = uiState.sidebarExpanded;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 origin = ImGui::GetWindowPos();

            dl->AddLine(ImVec2(origin.x + sidebarW - 1.0f, origin.y),
                        ImVec2(origin.x + sidebarW - 1.0f, origin.y + sidebarH),
                        IM_COL32(220, 229, 240, 255));

            ImVec2 logoMin = ImVec2(origin.x + 16.0f, origin.y + 16.0f);
            ImVec2 logoMax = ImVec2(origin.x + 52.0f, origin.y + 52.0f);
            drawSoftShadow(dl, logoMin, logoMax, 12.0f, IM_COL32(124, 58, 237, 30), ImVec2(0, 6));
            drawGradientRect(dl, logoMin, logoMax, 12.0f);
            dl->AddRect(ImVec2(logoMin.x + 9.0f, logoMin.y + 9.0f),
                        ImVec2(logoMax.x - 9.0f, logoMax.y - 9.0f),
                        IM_COL32(255, 255, 255, 190), 5.0f, 0, 1.3f);
            dl->AddCircleFilled(ImVec2((logoMin.x + logoMax.x) * 0.5f,
                                       (logoMin.y + logoMax.y) * 0.5f),
                                3.5f, IM_COL32(255, 255, 255, 255));

            if (expanded) {
                ImGui::SetCursorPos(ImVec2(64.0f, 18.0f));
                ImGui::PushFont(fontHeading());
                ImGui::TextColored(ColTextPrimary, "DataForge");
                ImGui::PopFont();
                ImGui::SetCursorPos(ImVec2(64.0f, 42.0f));
                ImGui::TextColored(ColTextFaint, "Workspace");
            }

            dl->AddLine(ImVec2(origin.x, origin.y + 68.0f),
                        ImVec2(origin.x + sidebarW, origin.y + 68.0f),
                        IM_COL32(220, 229, 240, 255));

            struct NavDef { NavItem item; const char* label; };
            static const NavDef navItems[] = {
                { NAV_OVERVIEW,  "Overview"  },
                { NAV_TASKS,     "My Tasks"  },
                { NAV_ANALYTICS, "Analytics" },
                { NAV_BENCHMARK, "Benchmark" }
            };

            ImGui::SetCursorPosY(84.0f);
            for (int i = 0; i < 4; ++i) {
                const NavDef& nav = navItems[i];
                bool active = uiState.activeNavItem == nav.item;
                ImVec2 btnPos = ImGui::GetCursorScreenPos();
                float btnH = 40.0f;
                float btnW = sidebarW - 22.0f;
                ImVec2 btnEnd = ImVec2(btnPos.x + btnW, btnPos.y + btnH);

                if (active) {
                    dl->AddRectFilled(btnPos, btnEnd, IM_COL32(238, 233, 255, 255), 14.0f);
                    dl->AddRectFilled(ImVec2(btnPos.x, btnPos.y + 8.0f),
                                      ImVec2(btnPos.x + 3.0f, btnPos.y + btnH - 8.0f),
                                      GradLeft, 4.0f);
                } else if (ImGui::IsMouseHoveringRect(btnPos, btnEnd)) {
                    dl->AddRectFilled(btnPos, btnEnd, IM_COL32(248, 250, 253, 255), 14.0f);
                }

                ImU32 iconCol = active ? IM_COL32(124, 58, 237, 255)
                                       : IM_COL32(100, 116, 139, 255);
                drawNavIcon(dl, ImVec2(btnPos.x + 20.0f, btnPos.y + btnH * 0.5f), nav.item, iconCol);

                if (expanded) {
                    ImGui::SetCursorScreenPos(ImVec2(btnPos.x + 38.0f, btnPos.y + 10.0f));
                    ImGui::PushFont(fontUiSemibold());
                    ImGui::TextColored(active ? HEX(0x6D28D9) : ColTextMuted, nav.label);
                    ImGui::PopFont();
                }

                ImGui::SetCursorScreenPos(btnPos);
                char id[32];
                std::snprintf(id, sizeof(id), "##nav%d", i);
                if (ImGui::InvisibleButton(id, ImVec2(btnW, btnH))) {
                    uiState.activeNavItem = nav.item;
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
            }

            int rootCount = static_cast<int>(logic::rootTaskIds(store).size());
            int totalTasks = static_cast<int>(store.tasks.size());
            if (expanded) {
                ImVec2 cardMin = ImVec2(origin.x + 12.0f, origin.y + sidebarH - 114.0f);
                ImVec2 cardMax = ImVec2(origin.x + sidebarW - 12.0f, origin.y + sidebarH - 56.0f);
                drawSoftShadow(dl, cardMin, cardMax, 14.0f);
                dl->AddRectFilled(cardMin, cardMax, IM_COL32(248, 250, 253, 255), 14.0f);
                dl->AddRect(cardMin, cardMax, IM_COL32(220, 229, 240, 255), 14.0f);
                ImGui::PushTextWrapPos(cardMax.x - 14.0f);
                ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 14.0f, cardMin.y + 12.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%d root projects", rootCount);
                ImGui::PopFont();
                ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 14.0f, cardMin.y + 32.0f));
                ImGui::TextColored(ColTextMuted, "%d tasks across the current store", totalTasks);
                ImGui::PopTextWrapPos();
            }

            ImGui::SetCursorPos(ImVec2(12.0f, sidebarH - 42.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_Text, ColTextMuted);
            if (ImGui::Button(expanded ? "Collapse" : "Expand", ImVec2(sidebarW - 24.0f, 30.0f))) {
                uiState.sidebarExpanded = !uiState.sidebarExpanded;
            }
            ImGui::PopStyleColor(4);

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);
        }

        void renderTopbar(const data::TaskStore& store, UiState& uiState,
                          float menuBarH, float sidebarW) {
            const ImGuiViewport* vp = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + sidebarW, vp->WorkPos.y + menuBarH));
            ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x - sidebarW, TOPBAR_H));

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNav;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ColBgCard);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28.0f, 0.0f));

            if (!ImGui::Begin("##Topbar", nullptr, flags)) {
                ImGui::End();
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                return;
            }

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 pos = ImGui::GetWindowPos();
            float width = ImGui::GetWindowWidth();

            const char* title = pageTitle(uiState.activeNavItem);
            const char* subtitle = pageSubtitle(uiState.activeNavItem);
            std::string topTitle = uiState.activeNavItem == NAV_OVERVIEW
                ? greetingTitle()
                : std::string(title);

            ImGui::SetCursorPos(ImVec2(8.0f, 11.0f));
            ImGui::PushFont(uiState.activeNavItem == NAV_OVERVIEW ? fontDisplay() : fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", topTitle.c_str());
            ImGui::PopFont();

            ImGui::SetCursorPos(ImVec2(8.0f, 42.0f));
            ImGui::TextColored(ColTextFaint,
                               uiState.activeNavItem == NAV_OVERVIEW
                                   ? "Here is what is on your plate today."
                                   : subtitle);

            data::Date today = logic::today();
            std::string todayText = logic::formatDate(today);

            ImVec2 chipMin = ImVec2(pos.x + width - 408.0f, pos.y + 20.0f);
            ImVec2 chipMax = ImVec2(pos.x + width - 252.0f, pos.y + 52.0f);
            dl->AddRectFilled(chipMin, chipMax, IM_COL32(247, 249, 252, 255), 16.0f);
            dl->AddRect(chipMin, chipMax, IM_COL32(220, 229, 240, 255), 16.0f);
            dl->AddCircle(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f), 5.0f, IM_COL32(149, 162, 180, 255), 0, 1.3f);
            dl->AddLine(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f),
                        ImVec2(chipMin.x + 18.0f, chipMin.y + 13.0f),
                        IM_COL32(149, 162, 180, 255), 1.2f);
            dl->AddLine(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f),
                        ImVec2(chipMin.x + 21.0f, chipMin.y + 17.0f),
                        IM_COL32(149, 162, 180, 255), 1.2f);
            ImGui::SetCursorScreenPos(ImVec2(chipMin.x + 32.0f, chipMin.y + 7.0f));
            ImGui::TextColored(ColTextMuted, "%s", todayText.empty() ? "Today" : todayText.c_str());

            ImVec2 saveMin = ImVec2(pos.x + width - 244.0f, pos.y + 20.0f);
            ImVec2 saveMax = ImVec2(pos.x + width - 168.0f, pos.y + 52.0f);
            ImU32 saveBg = store.dirty ? IM_COL32(255, 247, 237, 255) : IM_COL32(236, 253, 245, 255);
            ImU32 saveTx = store.dirty ? IM_COL32(180, 83, 9, 255) : IM_COL32(6, 95, 70, 255);
            dl->AddRectFilled(saveMin, saveMax, saveBg, 16.0f);
            ImGui::SetCursorScreenPos(ImVec2(saveMin.x + 14.0f, saveMin.y + 7.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(saveTx),
                               "%s", store.dirty ? "Unsaved" : "Ready");
            ImGui::PopFont();

            ImGui::SetCursorScreenPos(ImVec2(pos.x + width - 156.0f, pos.y + 18.0f));
            if (gradientButton("##newTaskTopbar", "+  New Task", ImVec2(148.0f, 36.0f), 16.0f)) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }

            dl->AddLine(ImVec2(pos.x, pos.y + TOPBAR_H - 1.0f),
                        ImVec2(pos.x + width, pos.y + TOPBAR_H - 1.0f),
                        IM_COL32(220, 229, 240, 255));

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        void renderMetricCard(const char* id, int value, const char* label,
                              const char* sub, ImU32 accent, float width) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ColBgCard);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 18.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::BeginChild(id, ImVec2(width, 104.0f), true);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetWindowPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 104.0f);
            drawSoftShadow(dl, min, max, 18.0f);
            dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, 255), 18.0f);
            dl->AddRect(min, max, IM_COL32(220, 229, 240, 255), 18.0f);
            dl->AddRectFilled(ImVec2(min.x + 18.0f, min.y + 18.0f),
                              ImVec2(min.x + 58.0f, min.y + 58.0f),
                              IM_COL32((accent >> IM_COL32_R_SHIFT) & 255,
                                       (accent >> IM_COL32_G_SHIFT) & 255,
                                       (accent >> IM_COL32_B_SHIFT) & 255, 20), 12.0f);
            dl->AddCircleFilled(ImVec2(min.x + 38.0f, min.y + 38.0f), 6.0f, accent);

            ImGui::PushTextWrapPos(width - 16.0f);
            ImGui::SetCursorPos(ImVec2(78.0f, 14.0f));
            ImGui::PushFont(fontDisplay());
            ImGui::TextColored(ColTextPrimary, "%d", value);
            ImGui::PopFont();
            ImGui::SetCursorPos(ImVec2(78.0f, 54.0f));
            ImGui::TextColored(ColTextMuted, "%s", label);
            if (sub != nullptr && sub[0] != '\0') {
                ImGui::SetCursorPos(ImVec2(78.0f, 74.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(accent), "%s", sub);
                ImGui::PopFont();
            }
            ImGui::PopTextWrapPos();

            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();
        }

        void renderOverviewTaskCard(const data::Task& task, UiState& uiState, float width) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 116.0f);
            UrgencyColor pc = colorForPriority(task.priority);
            ImU32 accent = IM_COL32((int)(pc.r * 255), (int)(pc.g * 255), (int)(pc.b * 255), 255);

            drawSoftShadow(dl, min, max, 18.0f);
            dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, 255), 18.0f);
            dl->AddRect(min, max,
                        uiState.selectedTaskId == task.id ? IM_COL32(124, 58, 237, 255)
                                                          : IM_COL32(220, 229, 240, 255),
                        18.0f, 0, 1.3f);
            dl->AddRectFilled(ImVec2(min.x + 18.0f, min.y + 18.0f),
                              ImVec2(min.x + 22.0f, max.y - 18.0f), accent, 99.0f);

            ImGui::PushTextWrapPos(max.x - 18.0f);
            ImGui::SetCursorScreenPos(ImVec2(min.x + 34.0f, min.y + 18.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", task.title.c_str());
            ImGui::PopFont();

            std::string note = task.description.empty() ? "No task notes yet." : task.description;
            if (note.size() > 72) {
                note = note.substr(0, 72) + "...";
            }
            ImGui::SetCursorScreenPos(ImVec2(min.x + 34.0f, min.y + 50.0f));
            ImGui::TextColored(ColTextFaint, "%s", note.c_str());
            ImGui::PopTextWrapPos();

            ImGui::SetCursorScreenPos(ImVec2(min.x + 34.0f, max.y - 34.0f));
            BadgeStyle sb = statusBadgeStyle(task.status);
            renderBadge(sb.label, sb.bg, sb.text, true, sb.dot);
            ImGui::SameLine(0, 8.0f);
            BadgeStyle pb = priorityBadgeStyle(task.priority);
            renderBadge(pb.label, pb.bg, pb.text, false, {});

            std::string due = logic::formatDate(task.deadline);
            ImGui::SetCursorScreenPos(ImVec2(max.x - 94.0f, max.y - 32.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextFaint, "%s", due.empty() ? "No due date" : due.c_str());
            ImGui::PopFont();

            ImGui::SetCursorScreenPos(min);
            char id[32];
            std::snprintf(id, sizeof(id), "##overviewTask%d", task.id);
            if (ImGui::InvisibleButton(id, ImVec2(width, 116.0f))) {
                uiState.selectedTaskId = task.id;
                uiState.activeNavItem = NAV_TASKS;
            }

            ImGui::Dummy(ImVec2(1.0f, 14.0f));
        }

        void renderOverviewPage(const data::TaskStore& store, UiState& uiState) {
            std::vector<int> roots = logic::rootTaskIds(store);
            std::vector<data::Task> focus = collectFocusTasks(store, 4);
            std::vector<const data::Task*> recent = collectRecentTasks(store, 4);

            int completed = 0;
            int blocked = 0;
            int inProgress = 0;
            for (std::size_t i = 0; i < store.tasks.size(); ++i) {
                if (store.tasks[i].status == data::STATUS_DONE) {
                    ++completed;
                } else if (store.tasks[i].status == data::STATUS_BLOCKED) {
                    ++blocked;
                } else if (store.tasks[i].status == data::STATUS_IN_PROGRESS) {
                    ++inProgress;
                }
            }

            float avail = ImGui::GetContentRegionAvail().x;
            float gap = 20.0f;
            float sideW = avail > 1180.0f ? 324.0f : 290.0f;
            float mainW = avail - sideW - gap;
            if (mainW < 520.0f) {
                sideW = 280.0f;
                mainW = avail - sideW - gap;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            ImGui::BeginChild("##OverviewMain", ImVec2(mainW, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);

            float contentW = ImGui::GetContentRegionAvail().x;
            float metricGap = 14.0f;
            float metricW = (contentW - metricGap * 3.0f) * 0.25f;
            renderMetricCard("##metricAll", static_cast<int>(store.tasks.size()),
                             "Total Tasks", inProgress > 0 ? "In progress now" : "Quiet queue",
                             IM_COL32(124, 58, 237, 255), metricW);
            ImGui::SameLine(0, metricGap);
            renderMetricCard("##metricDone", completed,
                             "Completed", "Shipping clean work",
                             IM_COL32(5, 150, 105, 255), metricW);
            ImGui::SameLine(0, metricGap);
            renderMetricCard("##metricRoot", static_cast<int>(roots.size()),
                             "Active Projects", "Root task trees",
                             IM_COL32(37, 99, 235, 255), metricW);
            ImGui::SameLine(0, metricGap);
            renderMetricCard("##metricBlocked", blocked,
                             "Blocked", blocked > 0 ? "Needs attention" : "No blockers",
                             IM_COL32(220, 38, 38, 255), metricW);

            ImGui::Dummy(ImVec2(1.0f, 18.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Today's Focus");
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::SetCursorPosX(mainW - 90.0f);
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(HEX(0x7C3AED), "+ Add task");
            ImGui::PopFont();
            if (ImGui::IsItemClicked()) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            ImGui::Separator();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (focus.empty()) {
                ImGui::TextColored(ColTextFaint, "No active tasks yet. Create one to start the board.");
            } else {
                for (std::size_t i = 0; i < focus.size(); ++i) {
                    renderOverviewTaskCard(focus[i], uiState, contentW);
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();

            ImGui::SameLine(0, gap);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            ImGui::BeginChild("##OverviewSide", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);

            float sideContentW = ImGui::GetContentRegionAvail().x;

            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Active Projects");
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (roots.empty()) {
                ImGui::TextColored(ColTextFaint, "No root projects yet.");
            } else {
                for (std::size_t i = 0; i < roots.size() && i < 4; ++i) {
                    const data::Task* root = findTask(store, roots[i]);
                    if (root == nullptr) {
                        continue;
                    }

                    ImVec2 min = ImGui::GetCursorScreenPos();
                    ImVec2 max = ImVec2(min.x + sideContentW, min.y + 72.0f);
                    drawSoftShadow(ImGui::GetWindowDrawList(), min, max, 18.0f);
                    ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(255, 255, 255, 255), 18.0f);
                    ImGui::GetWindowDrawList()->AddRect(min, max, IM_COL32(220, 229, 240, 255), 18.0f);

                    UrgencyColor pc = colorForPriority(root->priority);
                    ImU32 accent = IM_COL32((int)(pc.r * 255), (int)(pc.g * 255), (int)(pc.b * 255), 255);
                    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(min.x + 18.0f, min.y + 24.0f), 4.0f, accent);
                    ImGui::PushTextWrapPos(max.x - 44.0f);
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 32.0f, min.y + 15.0f));
                    ImGui::PushFont(fontUiSemibold());
                    ImGui::TextColored(ColTextSecondary, "%s", root->title.c_str());
                    ImGui::PopFont();

                    float completion = logic::calculateWeightedCompletion(store, root->id);
                    renderProgressRing(ImVec2(max.x - 26.0f, min.y + 27.0f), 12.0f, 3.0f,
                                       completion * 100.0f, accent);
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 32.0f, min.y + 40.0f));
                    ImGui::TextColored(ColTextFaint, "%d subtasks  |  %.0f%% done",
                                       logic::countDescendants(store, root->id),
                                       completion * 100.0f);
                    ImGui::PopTextWrapPos();

                    ImGui::SetCursorScreenPos(min);
                    char id[32];
                    std::snprintf(id, sizeof(id), "##rootCard%d", root->id);
                    if (ImGui::InvisibleButton(id, ImVec2(sideContentW, 72.0f))) {
                        uiState.selectedTaskId = root->id;
                        uiState.activeNavItem = NAV_TASKS;
                    }
                    ImGui::Dummy(ImVec2(1.0f, 14.0f));
                }
            }

            ImGui::Dummy(ImVec2(1.0f, 12.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Recent Activity");
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (recent.empty()) {
                ImGui::TextColored(ColTextFaint, "Nothing recent yet.");
            } else {
                for (std::size_t i = 0; i < recent.size(); ++i) {
                    const data::Task& task = *recent[i];
                    ImVec2 min = ImGui::GetCursorScreenPos();
                    ImVec2 max = ImVec2(min.x + sideContentW, min.y + 64.0f);
                    ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, max.y),
                                                        ImVec2(max.x, max.y),
                                                        IM_COL32(228, 234, 242, 255));

                    BadgeStyle badge = statusBadgeStyle(task.status);
                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, min.y + 6.0f),
                                                              ImVec2(min.x + 32.0f, min.y + 38.0f),
                                                              ImGui::ColorConvertFloat4ToU32(badge.bg), 10.0f);
                    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(min.x + 16.0f, min.y + 22.0f),
                                                                4.0f, ImGui::ColorConvertFloat4ToU32(badge.dot));
                    ImGui::PushTextWrapPos(max.x - 8.0f);
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 44.0f, min.y + 6.0f));
                    ImGui::TextColored(ColTextSecondary, "%s", task.title.c_str());
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 44.0f, min.y + 28.0f));
                    ImGui::TextColored(ColTextFaint, "%s",
                                       logic::formatDate(task.updatedAt).empty()
                                           ? "Updated recently"
                                           : logic::formatDate(task.updatedAt).c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::Dummy(ImVec2(1.0f, 64.0f));
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        void renderTaskWorkspace(data::TaskStore& store, UiState& uiState) {
            float avail = ImGui::GetContentRegionAvail().x;
            float treeW = avail * 0.23f;
            float listW = avail * 0.43f;
            float detailsW = avail - treeW - listW - 16.0f;
            if (treeW < 220.0f) treeW = 220.0f;
            if (listW < 360.0f) listW = 360.0f;
            if (detailsW < 290.0f) detailsW = 290.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 20.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImGui::BeginChild("##TreePane", ImVec2(treeW, 0.0f), true);
            renderTaskTreePanel(store, uiState);
            ImGui::EndChild();

            ImGui::SameLine(0, 8.0f);
            ImGui::BeginChild("##TaskPane", ImVec2(listW, 0.0f), true);
            renderTaskTablePanel(store, uiState);
            ImGui::EndChild();

            ImGui::SameLine(0, 8.0f);
            ImGui::BeginChild("##DetailPane", ImVec2(0.0f, 0.0f), true);
            renderTaskDetailsPanel(store, uiState);
            ImGui::EndChild();

            ImGui::PopStyleVar(2);
        }

        void renderMainShell(data::TaskStore& store, UiState& uiState,
                             float menuBarH, float sidebarW) {
            const ImGuiViewport* vp = ImGui::GetMainViewport();
            float contentX = vp->WorkPos.x + sidebarW;
            float contentY = vp->WorkPos.y + menuBarH + TOPBAR_H;
            float contentW = vp->WorkSize.x - sidebarW;
            float contentH = vp->WorkSize.y - menuBarH - TOPBAR_H - STATUSBAR_H;

            ImGui::SetNextWindowPos(ImVec2(contentX, contentY));
            ImGui::SetNextWindowSize(ImVec2(contentW, contentH));

            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24.0f, 20.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ColBgBase);

            if (!ImGui::Begin("##DataForgeShell", nullptr, flags)) {
                ImGui::End();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                return;
            }

            if (uiState.showStatsPanel) {
                uiState.activeNavItem = NAV_ANALYTICS;
                uiState.showStatsPanel = false;
            }
            if (uiState.showBenchmarkPanel) {
                uiState.activeNavItem = NAV_BENCHMARK;
                uiState.showBenchmarkPanel = false;
            }

            switch (uiState.activeNavItem) {
                case NAV_OVERVIEW:
                    renderOverviewPage(store, uiState);
                    break;
                case NAV_TASKS:
                    renderTaskWorkspace(store, uiState);
                    break;
                case NAV_ANALYTICS:
                    renderStatsPanel(store, uiState);
                    break;
                case NAV_BENCHMARK:
                    renderBenchmarkPanel(uiState);
                    break;
            }

            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

    }

    void renderApp(data::TaskStore& store, UiState& uiState) {
        processKeyboardShortcuts(store, uiState);
        renderMenuBar(store, uiState);

        float menuBarH = ImGui::GetFrameHeight();
        float sidebarW = uiState.sidebarExpanded ? SIDEBAR_W_FULL : SIDEBAR_W_SLIM;

        renderSidebar(store, uiState, menuBarH, sidebarW);
        renderTopbar(store, uiState, menuBarH, sidebarW);
        renderMainShell(store, uiState, menuBarH, sidebarW);
        renderStatusBar(store, uiState);

        renderAddEditDialog(store, uiState);
        renderConfirmDeleteDialog(store, uiState);
        renderDirtyExitConfirmDialog(store, uiState);
        renderAboutPopup(uiState);
        renderToast(uiState);
    }

}
