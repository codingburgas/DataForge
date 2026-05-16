#include "ui/app_shell.h"
#include "ui/menu_bar.h"
#include "ui/task_tree.h"
#include "ui/task_table.h"
#include "ui/task_details.h"
#include "ui/dialogs.h"
#include "ui/stats_panel.h"
#include "ui/benchmark_panel.h"
#include "ui/help_panel.h"
#include "ui/history_panel.h"
#include "ui/productivity_panel.h"
#include "ui/status_bar.h"
#include "ui/toast.h"
#include "ui/shortcuts.h"
#include "ui/theme.h"
#include "ui/i18n.h"
#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "logic/sort.h"
#include "data/store.h"
#include "imgui.h"
#include <cmath>

namespace ui {

    namespace {

        static const float TOPBAR_H        = 76.0f;
        static const float STATUSBAR_H     = 34.0f;
        static const float SIDEBAR_W_FULL  = 244.0f;
        static const float SIDEBAR_W_SLIM  = 76.0f;

        std::vector<data::Task> collectFocusTasks(const data::TaskStore& store, std::size_t limit) {
            std::vector<data::Task> out;
            for (const data::Task& t : store.tasks) {
                if (t.status != data::STATUS_DONE) {
                    out.push_back(t);
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
            for (const data::Task& t : store.tasks) {
                out.push_back(&t);
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
                case NAV_OVERVIEW:     return tr(K_PAGE_OVERVIEW);
                case NAV_TASKS:        return tr(K_PAGE_TASKS);
                case NAV_ANALYTICS:    return tr(K_PAGE_ANALYTICS);
                case NAV_BENCHMARK:    return tr(K_PAGE_BENCHMARK);
                case NAV_HELP:         return tr(K_PAGE_HELP);
                case NAV_HISTORY:      return "History";
                case NAV_PRODUCTIVITY: return "Productivity";
            }
            return tr(K_PAGE_WORKSPACE);
        }

        const char* pageSubtitle(NavItem item) {
            switch (item) {
                case NAV_OVERVIEW:     return tr(K_SUB_OVERVIEW);
                case NAV_TASKS:        return tr(K_SUB_TASKS);
                case NAV_ANALYTICS:    return tr(K_SUB_ANALYTICS);
                case NAV_BENCHMARK:    return tr(K_SUB_BENCHMARK);
                case NAV_HELP:         return tr(K_SUB_HELP);
                case NAV_HISTORY:      return "Audit trail of every store mutation.";
                case NAV_PRODUCTIVITY: return "Levels, badges, and streaks driven by your activity.";
            }
            return "";
        }

        std::string greetingTitle() {
            std::time_t now = std::time(nullptr);
            std::tm local{};
            localtime_s(&local, &now);
            if (local.tm_hour < 12) {
                return tr(K_GREETING_MORNING);
            }
            if (local.tm_hour < 18) {
                return tr(K_GREETING_AFTERNOON);
            }
            return tr(K_GREETING_EVENING);
        }

        ImU32 u32(ImVec4 color) {
            return ImGui::ColorConvertFloat4ToU32(color);
        }

        void drawShellBackdrop(ImDrawList* dl, ImVec2 min, ImVec2 max) {
            if (dl == nullptr) {
                return;
            }

            ImU32 topLeft = u32(isDarkTheme() ? HEX(0x18233A) : HEX(0xF8FAFF));
            ImU32 topRight = u32(isDarkTheme() ? HEX(0x111827) : HEX(0xFFF7ED));
            ImU32 bottom = u32(ColBgBase);
            dl->AddRectFilledMultiColor(min, max, topLeft, topRight, bottom, bottom);

            ImU32 accentWash = isDarkTheme()
                ? IM_COL32(124, 58, 237, 20)
                : IM_COL32(124, 58, 237, 18);
            ImU32 warmWash = isDarkTheme()
                ? IM_COL32(249, 115, 22, 14)
                : IM_COL32(249, 115, 22, 18);
            dl->AddCircleFilled(ImVec2(max.x - 90.0f, min.y + 74.0f), 142.0f, accentWash, 64);
            dl->AddCircleFilled(ImVec2(min.x + 180.0f, max.y - 34.0f), 120.0f, warmWash, 64);
        }

        void drawNavIcon(ImDrawList* dl, ImVec2 centre, NavItem item, ImU32 color) {
            switch (item) {
                case NAV_HISTORY:
                    dl->AddCircle(ImVec2(centre.x, centre.y), 7.0f, color, 0, 1.6f);
                    dl->AddLine(ImVec2(centre.x, centre.y),
                                ImVec2(centre.x, centre.y - 4.0f), color, 1.6f);
                    dl->AddLine(ImVec2(centre.x, centre.y),
                                ImVec2(centre.x + 4.0f, centre.y), color, 1.6f);
                    break;
                case NAV_PRODUCTIVITY:
                    dl->AddTriangleFilled(ImVec2(centre.x,        centre.y - 7.0f),
                                          ImVec2(centre.x + 6.0f, centre.y + 4.0f),
                                          ImVec2(centre.x - 6.0f, centre.y + 4.0f),
                                          color);
                    break;
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
                case NAV_HELP:
                    dl->AddCircle(ImVec2(centre.x, centre.y), 7.0f, color, 0, 1.5f);
                    dl->AddLine(ImVec2(centre.x, centre.y - 3.0f),
                                ImVec2(centre.x, centre.y + 1.0f), color, 1.6f);
                    dl->AddCircleFilled(ImVec2(centre.x, centre.y + 4.0f), 1.2f, color);
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
                        cardBorderU32());

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
                ImGui::TextColored(ColTextFaint, "%s", tr(K_SIDEBAR_LOGO_SUB));
            }

            dl->AddLine(ImVec2(origin.x, origin.y + 68.0f),
                        ImVec2(origin.x + sidebarW, origin.y + 68.0f),
                        cardBorderU32());

            struct NavDef { NavItem item; const char* label; };
            const NavDef navItems[] = {
                { NAV_OVERVIEW,     tr(K_NAV_OVERVIEW)  },
                { NAV_TASKS,        tr(K_NAV_TASKS)     },
                { NAV_ANALYTICS,    tr(K_NAV_ANALYTICS) },
                { NAV_PRODUCTIVITY, "Productivity"      },
                { NAV_HISTORY,      "History"           },
                { NAV_BENCHMARK,    tr(K_NAV_BENCHMARK) },
                { NAV_HELP,         tr(K_NAV_HELP)      }
            };
            const int navItemCount = static_cast<int>(IM_ARRAYSIZE(navItems));

            const float NAV_BTN_H   = 44.0f;
            const float NAV_BTN_GAP = 6.0f;
            const float NAV_BASE_Y  = origin.y + 84.0f;
            const float NAV_BTN_W   = sidebarW - 22.0f;
            const float NAV_LEFT_X  = origin.x + 11.0f;

            int activeIdx = 0;
            for (int i = 0; i < navItemCount; ++i) {
                if (navItems[i].item == uiState.activeNavItem) { activeIdx = i; break; }
            }
            float targetIndY = NAV_BASE_Y + activeIdx * (NAV_BTN_H + NAV_BTN_GAP);

            ImGuiStorage* navStore = ImGui::GetStateStorage();
            ImGuiID indKey = ImGui::GetID("##navIndY");
            float curIndY = navStore->GetFloat(indKey, targetIndY);
            float navDt   = ImGui::GetIO().DeltaTime;
            float lerpK   = 1.0f - std::exp(-navDt * 16.0f);
            curIndY += (targetIndY - curIndY) * lerpK;
            if (std::fabs(curIndY - targetIndY) < 0.25f) curIndY = targetIndY;
            navStore->SetFloat(indKey, curIndY);

            {
                ImVec2 indMin = ImVec2(NAV_LEFT_X, curIndY);
                ImVec2 indMax = ImVec2(NAV_LEFT_X + NAV_BTN_W, curIndY + NAV_BTN_H);
                drawSoftShadow(dl, indMin, indMax, 12.0f,
                               IM_COL32(124, 58, 237, 110), ImVec2(0.0f, 10.0f));
                drawSoftShadow(dl, indMin, indMax, 12.0f,
                               IM_COL32(124, 58, 237, 70), ImVec2(0.0f, 4.0f));
                drawGradientRect(dl, indMin, indMax, 12.0f, GradLeft, GradMid);
                dl->AddRectFilled(ImVec2(indMin.x, indMin.y + 6.0f),
                                  ImVec2(indMin.x + 4.0f, indMax.y - 6.0f),
                                  IM_COL32(255, 255, 255, 220), 99.0f);
                dl->AddLine(ImVec2(indMin.x + 8.0f, indMin.y + 1.0f),
                            ImVec2(indMax.x - 8.0f, indMin.y + 1.0f),
                            IM_COL32(255, 255, 255, 80), 1.0f);
                dl->AddLine(ImVec2(indMin.x + 8.0f, indMax.y - 1.0f),
                            ImVec2(indMax.x - 8.0f, indMax.y - 1.0f),
                            IM_COL32(0, 0, 0, 60), 1.0f);
            }

            ImGui::SetCursorPosY(84.0f);
            for (int i = 0; i < navItemCount; ++i) {
                ImGui::SetCursorPosX(11.0f);
                const NavDef& nav = navItems[i];
                bool active = uiState.activeNavItem == nav.item;
                ImVec2 btnPos = ImGui::GetCursorScreenPos();
                float btnH = NAV_BTN_H;
                float btnW = NAV_BTN_W;
                ImVec2 btnEnd = ImVec2(btnPos.x + btnW, btnPos.y + btnH);

                char id[32];
                std::snprintf(id, sizeof(id), "##nav%d", i);
                ImGui::SetCursorScreenPos(btnPos);
                ImGui::InvisibleButton(id, ImVec2(btnW, btnH));
                bool hovered = ImGui::IsItemHovered();
                bool held    = ImGui::IsItemActive();
                bool clicked = ImGui::IsItemClicked();
                float anim = animateHover(id, hovered);

                float lift = active ? 0.0f : (anim * 5.0f - (held ? 2.0f : 0.0f));
                if (lift < 0.0f) lift = 0.0f;
                ImVec2 drawMin = ImVec2(btnPos.x, btnPos.y - lift);
                ImVec2 drawMax = ImVec2(btnEnd.x, btnEnd.y - lift);

                if (!active && anim > 0.001f) {
                    drawSoftShadow(dl, drawMin, drawMax, 12.0f,
                                   IM_COL32(15, 23, 42, (int)(70 * anim)),
                                   ImVec2(0.0f, 4.0f + 8.0f * anim));
                    int alpha = (int)(anim * 200.0f);
                    ImU32 hoverBg = IM_COL32((int)(ColBgHover.x * 255), (int)(ColBgHover.y * 255),
                                             (int)(ColBgHover.z * 255), alpha);
                    dl->AddRectFilled(drawMin, drawMax, hoverBg, 12.0f);
                    int glow = (int)(anim * 28.0f);
                    dl->AddRectFilled(drawMin, drawMax,
                                      IM_COL32(167, 139, 250, glow), 12.0f);
                    dl->AddLine(ImVec2(drawMin.x + 8.0f, drawMin.y + 1.0f),
                                ImVec2(drawMax.x - 8.0f, drawMin.y + 1.0f),
                                IM_COL32(255, 255, 255, (int)(90 * anim)), 1.0f);
                    dl->AddLine(ImVec2(drawMin.x + 8.0f, drawMax.y - 1.0f),
                                ImVec2(drawMax.x - 8.0f, drawMax.y - 1.0f),
                                IM_COL32(0, 0, 0, (int)(55 * anim)), 1.0f);
                }

                char pkBuf[32];
                std::snprintf(pkBuf, sizeof(pkBuf), "##navPulse%d", i);
                ImGuiID pulseKey = ImGui::GetID(pkBuf);
                float pulseStart = navStore->GetFloat(pulseKey, -1.0f);
                float now = (float)ImGui::GetTime();
                float pulseDur = 0.45f;
                if (pulseStart > 0.0f) {
                    float pe = now - pulseStart;
                    if (pe >= 0.0f && pe < pulseDur) {
                        float pt = pe / pulseDur;
                        float radius = 10.0f + pt * 70.0f;
                        int   a      = (int)((1.0f - pt) * (1.0f - pt) * 110.0f);
                        ImVec2 c     = ImVec2(btnPos.x + 22.0f, btnPos.y + btnH * 0.5f - lift);
                        dl->AddCircle(c, radius,
                                      IM_COL32(167, 139, 250, a), 32, 2.0f);
                        dl->AddCircle(c, radius * 0.6f,
                                      IM_COL32(255, 255, 255, a / 2), 32, 1.0f);
                    } else if (pe >= pulseDur) {
                        navStore->SetFloat(pulseKey, -1.0f);
                    }
                }

                ImU32 iconCol = active ? IM_COL32(255, 255, 255, 255)
                                       : (hovered ? u32(ColAccent)
                                                  : u32(ColTextMuted));
                drawNavIcon(dl, ImVec2(btnPos.x + 22.0f, btnPos.y + btnH * 0.5f - lift),
                            nav.item, iconCol);

                if (expanded) {
                    ImGui::SetCursorScreenPos(ImVec2(btnPos.x + 42.0f, btnPos.y + 12.0f - lift));
                    ImGui::PushFont(fontUiSemibold());
                    ImVec4 lblCol = active ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
                                           : (hovered ? ColTextPrimary : ColTextSecondary);
                    ImGui::TextColored(lblCol, "%s", nav.label);
                    ImGui::PopFont();
                }

                if (clicked) {
                    uiState.activeNavItem = nav.item;
                    navStore->SetFloat(pulseKey, now);
                }

                ImGui::SetCursorScreenPos(ImVec2(btnPos.x, btnPos.y + btnH + NAV_BTN_GAP));
            }

            int rootCount = static_cast<int>(logic::rootTaskIds(store).size());
            int totalTasks = static_cast<int>(store.tasks.size());
            if (expanded) {
                ImVec2 cardMin = ImVec2(origin.x + 12.0f, origin.y + sidebarH - 114.0f);
                ImVec2 cardMax = ImVec2(origin.x + sidebarW - 12.0f, origin.y + sidebarH - 56.0f);
                drawSoftShadow(dl, cardMin, cardMax, 14.0f);
                dl->AddRectFilled(cardMin, cardMax, ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 14.0f);
                dl->AddRect(cardMin, cardMax, cardBorderU32(), 14.0f);
                ImGui::PushTextWrapPos(cardMax.x - 14.0f);
                ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 14.0f, cardMin.y + 12.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, tr(K_SIDEBAR_ROOT_PROJECTS_FMT), rootCount);
                ImGui::PopFont();
                ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 14.0f, cardMin.y + 32.0f));
                ImGui::TextColored(ColTextMuted, tr(K_SIDEBAR_TASKS_ACROSS_FMT), totalTasks);
                ImGui::PopTextWrapPos();
            }

            ImGui::SetCursorPos(ImVec2(12.0f, sidebarH - 42.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ColBgSubtle);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColBgHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColBgActive);
            ImGui::PushStyleColor(ImGuiCol_Text, ColTextMuted);
            if (ImGui::Button(expanded ? tr(K_SIDEBAR_COLLAPSE) : tr(K_SIDEBAR_EXPAND), ImVec2(sidebarW - 24.0f, 30.0f))) {
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

            float actionW = width > 560.0f ? 148.0f : 118.0f;
            float rightReserve = actionW + 32.0f;
            if (width > 760.0f) {
                rightReserve += 252.0f;
            } else if (width > 560.0f) {
                rightReserve += 94.0f;
            }
            float titleWrap = (std::max)(140.0f, width - rightReserve);

            ImGui::SetCursorPos(ImVec2(8.0f, 11.0f));
            ImGui::PushFont(uiState.activeNavItem == NAV_OVERVIEW ? fontDisplay() : fontHeading());
            ImGui::PushTextWrapPos(titleWrap);
            ImGui::TextColored(ColTextPrimary, "%s", topTitle.c_str());
            ImGui::PopTextWrapPos();
            ImGui::PopFont();

            ImGui::SetCursorPos(ImVec2(8.0f, 42.0f));
            ImGui::PushTextWrapPos(titleWrap);
            ImGui::TextColored(ColTextFaint, "%s",
                               uiState.activeNavItem == NAV_OVERVIEW
                                   ? tr(K_OVERVIEW_TOPBAR_SUB)
                                   : subtitle);
            ImGui::PopTextWrapPos();

            data::Date today = logic::today();
            std::string todayText = logic::formatDate(today);

            float cursorRight = pos.x + width - 8.0f;
            ImGui::SetCursorScreenPos(ImVec2(cursorRight - actionW, pos.y + 18.0f));
            if (gradientButton("##newTaskTopbar", tr(K_NEW_TASK_BTN), ImVec2(actionW, 36.0f), 16.0f)) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            cursorRight -= actionW + 12.0f;

            if (width > 560.0f) {
                ImVec2 saveMin = ImVec2(cursorRight - 76.0f, pos.y + 20.0f);
                ImVec2 saveMax = ImVec2(cursorRight, pos.y + 52.0f);
                ImU32 saveBg = isDarkTheme()
                    ? (store.dirty ? IM_COL32(63, 41, 14, 255) : IM_COL32(14, 50, 36, 255))
                    : (store.dirty ? IM_COL32(255, 247, 237, 255) : IM_COL32(236, 253, 245, 255));
                ImU32 saveTx = isDarkTheme()
                    ? (store.dirty ? IM_COL32(252, 187, 102, 255) : IM_COL32(110, 220, 170, 255))
                    : (store.dirty ? IM_COL32(180, 83, 9, 255) : IM_COL32(6, 95, 70, 255));
                dl->AddRectFilled(saveMin, saveMax, saveBg, 16.0f);
                ImGui::SetCursorScreenPos(ImVec2(saveMin.x + 14.0f, saveMin.y + 7.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(saveTx),
                                   "%s", store.dirty ? tr(K_STATE_UNSAVED) : tr(K_STATE_READY));
                ImGui::PopFont();
                cursorRight -= 88.0f;
            }

            if (width > 760.0f) {
                ImVec2 chipMin = ImVec2(cursorRight - 156.0f, pos.y + 20.0f);
                ImVec2 chipMax = ImVec2(cursorRight, pos.y + 52.0f);
                dl->AddRectFilled(chipMin, chipMax, ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 16.0f);
                dl->AddRect(chipMin, chipMax, cardBorderU32(), 16.0f);
                dl->AddCircle(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f), 5.0f, u32(ColTextFaint), 0, 1.3f);
                dl->AddLine(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f),
                            ImVec2(chipMin.x + 18.0f, chipMin.y + 13.0f),
                            u32(ColTextFaint), 1.2f);
                dl->AddLine(ImVec2(chipMin.x + 18.0f, chipMin.y + 16.0f),
                            ImVec2(chipMin.x + 21.0f, chipMin.y + 17.0f),
                            u32(ColTextFaint), 1.2f);
                ImGui::SetCursorScreenPos(ImVec2(chipMin.x + 32.0f, chipMin.y + 7.0f));
                ImGui::TextColored(ColTextMuted, "%s", todayText.empty() ? tr(K_TODAY_LABEL) : todayText.c_str());
            }

            dl->AddLine(ImVec2(pos.x, pos.y + TOPBAR_H - 1.0f),
                        ImVec2(pos.x + width, pos.y + TOPBAR_H - 1.0f),
                        cardBorderU32());

            ImGui::End();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        void renderMetricCard(const char* id, int value, const char* label,
                              const char* sub, ImU32 accent, float width) {
            ImVec2 origin = ImGui::GetCursorScreenPos();
            ImVec2 min    = origin;
            ImVec2 max    = ImVec2(min.x + width, min.y + 104.0f);
            CardVisual cv = drawCardChrome(id, min, max, 18.0f, 6.0f);
            ImDrawList* dl = ImGui::GetWindowDrawList();

            dl->AddRectFilled(ImVec2(cv.drawMin.x + 18.0f, cv.drawMin.y + 18.0f),
                              ImVec2(cv.drawMin.x + 58.0f, cv.drawMin.y + 58.0f),
                              IM_COL32((accent >> IM_COL32_R_SHIFT) & 255,
                                       (accent >> IM_COL32_G_SHIFT) & 255,
                                       (accent >> IM_COL32_B_SHIFT) & 255, 20), 12.0f);
            dl->AddCircleFilled(ImVec2(cv.drawMin.x + 38.0f, cv.drawMin.y + 38.0f), 6.0f, accent);

            ImGui::PushTextWrapPos(cv.drawMin.x + width - 16.0f);
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 78.0f, cv.drawMin.y + 14.0f));
            ImGui::PushFont(fontDisplay());
            ImGui::TextColored(ColTextPrimary, "%d", value);
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 78.0f, cv.drawMin.y + 54.0f));
            ImGui::TextColored(ColTextMuted, "%s", label);
            if (sub != nullptr && sub[0] != '\0') {
                ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 78.0f, cv.drawMin.y + 74.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(accent), "%s", sub);
                ImGui::PopFont();
            }
            ImGui::PopTextWrapPos();

            ImGui::SetCursorScreenPos(min);
            ImGui::Dummy(ImVec2(width, 104.0f));
        }

        void renderOverviewTaskCard(const data::Task& task, UiState& uiState, float width) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 116.0f);
            UrgencyColor pc = colorForPriority(task.priority);
            ImU32 accent = urgencyToImU32(pc);
            char id[32];
            std::snprintf(id, sizeof(id), "##overviewTask%d", task.id);

            CardVisual cv = drawCardChrome(id, min, max, 18.0f, 7.0f);
            if (uiState.selectedTaskId == task.id) {
                dl->AddRect(cv.drawMin, cv.drawMax, u32(ColAccent), 18.0f, 0, 1.6f);
            }
            dl->AddRectFilled(ImVec2(cv.drawMin.x + 18.0f, cv.drawMin.y + 18.0f),
                              ImVec2(cv.drawMin.x + 22.0f, cv.drawMax.y - 18.0f),
                              accent, 99.0f);

            ImGui::PushTextWrapPos(cv.drawMax.x - 18.0f);
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 34.0f, cv.drawMin.y + 18.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", task.title.c_str());
            ImGui::PopFont();

            std::string note = task.description.empty() ? std::string(tr(K_OV_NO_TASK_NOTES)) : task.description;
            if (note.size() > 72) {
                note = note.substr(0, 72) + "...";
            }
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 34.0f, cv.drawMin.y + 50.0f));
            ImGui::TextColored(ColTextFaint, "%s", note.c_str());
            ImGui::PopTextWrapPos();

            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 34.0f, cv.drawMax.y - 34.0f));
            BadgeStyle sb = statusBadgeStyle(task.status);
            renderBadge(sb.label, sb.bg, sb.text, true, sb.dot);
            ImGui::SameLine(0, 8.0f);
            BadgeStyle pb = priorityBadgeStyle(task.priority);
            renderBadge(pb.label, pb.bg, pb.text, false, {});

            std::string due = logic::formatDate(task.deadline);
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMax.x - 94.0f, cv.drawMax.y - 32.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextFaint, "%s", due.empty() ? tr(K_OV_NO_DUE) : due.c_str());
            ImGui::PopFont();

            ImGui::SetCursorScreenPos(min);
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
            for (const data::Task& t : store.tasks) {
                if (t.status == data::STATUS_DONE) {
                    ++completed;
                } else if (t.status == data::STATUS_BLOCKED) {
                    ++blocked;
                } else if (t.status == data::STATUS_IN_PROGRESS) {
                    ++inProgress;
                }
            }

            float avail = ImGui::GetContentRegionAvail().x;
            float availH = ImGui::GetContentRegionAvail().y;
            float gap = 20.0f;
            bool stacked = avail < 980.0f;
            float sideW = stacked ? avail : (avail > 1180.0f ? 324.0f : 290.0f);
            float mainW = stacked ? avail : avail - sideW - gap;
            if (!stacked && mainW < 520.0f) {
                sideW = 280.0f;
                mainW = avail - sideW - gap;
            }
            float mainH = stacked ? (std::max)(360.0f, availH * 0.58f) : 0.0f;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            ImGui::BeginChild("##OverviewMain", ImVec2(mainW, mainH), ImGuiChildFlags_AlwaysUseWindowPadding);

            float contentW = ImGui::GetContentRegionAvail().x;
            float metricGap = 14.0f;
            if (contentW < 760.0f) {
                float metricW = (contentW - metricGap) * 0.5f;
                renderMetricCard("##metricAll", static_cast<int>(store.tasks.size()),
                                 tr(K_OV_TOTAL_TASKS), inProgress > 0 ? tr(K_OV_IN_PROGRESS_NOW) : tr(K_OV_QUIET_QUEUE),
                                 u32(ColAccent), metricW);
                ImGui::SameLine(0, metricGap);
                renderMetricCard("##metricDone", completed,
                                 tr(K_OV_COMPLETED), tr(K_OV_SHIPPING_CLEAN),
                                 IM_COL32(5, 150, 105, 255), metricW);
                ImGui::Dummy(ImVec2(1.0f, metricGap));
                renderMetricCard("##metricRoot", static_cast<int>(roots.size()),
                                 tr(K_OV_ACTIVE_PROJECTS), tr(K_OV_ROOT_TREES),
                                 IM_COL32(37, 99, 235, 255), metricW);
                ImGui::SameLine(0, metricGap);
                renderMetricCard("##metricBlocked", blocked,
                                 tr(K_OV_BLOCKED), blocked > 0 ? tr(K_OV_NEEDS_ATTENTION) : tr(K_OV_NO_BLOCKERS),
                                 IM_COL32(220, 38, 38, 255), metricW);
            } else {
                float metricW = (contentW - metricGap * 3.0f) * 0.25f;
                renderMetricCard("##metricAll", static_cast<int>(store.tasks.size()),
                                 tr(K_OV_TOTAL_TASKS), inProgress > 0 ? tr(K_OV_IN_PROGRESS_NOW) : tr(K_OV_QUIET_QUEUE),
                                 u32(ColAccent), metricW);
                ImGui::SameLine(0, metricGap);
                renderMetricCard("##metricDone", completed,
                                 tr(K_OV_COMPLETED), tr(K_OV_SHIPPING_CLEAN),
                                 IM_COL32(5, 150, 105, 255), metricW);
                ImGui::SameLine(0, metricGap);
                renderMetricCard("##metricRoot", static_cast<int>(roots.size()),
                                 tr(K_OV_ACTIVE_PROJECTS), tr(K_OV_ROOT_TREES),
                                 IM_COL32(37, 99, 235, 255), metricW);
                ImGui::SameLine(0, metricGap);
                renderMetricCard("##metricBlocked", blocked,
                                 tr(K_OV_BLOCKED), blocked > 0 ? tr(K_OV_NEEDS_ATTENTION) : tr(K_OV_NO_BLOCKERS),
                                 IM_COL32(220, 38, 38, 255), metricW);
            }

            ImGui::Dummy(ImVec2(1.0f, 18.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", tr(K_OV_TODAYS_FOCUS));
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::SetCursorPosX(mainW - 90.0f);
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(HEX(0x7C3AED), "%s", tr(K_OV_ADD_TASK));
            ImGui::PopFont();
            if (ImGui::IsItemClicked()) {
                openCreateDialog(uiState, uiState.selectedTaskId);
            }
            ImGui::Separator();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (focus.empty()) {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_OV_NO_ACTIVE));
            } else {
                for (const data::Task& t : focus) {
                    renderOverviewTaskCard(t, uiState, contentW);
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();

            if (stacked) {
                ImGui::Dummy(ImVec2(1.0f, gap));
            } else {
                ImGui::SameLine(0, gap);
            }
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
            ImGui::BeginChild("##OverviewSide", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);

            float sideContentW = ImGui::GetContentRegionAvail().x;

            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", tr(K_OV_ACTIVE_PROJECTS_HEADING));
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (roots.empty()) {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_OV_NO_ROOT_PROJECTS));
            } else {
                for (std::size_t i = 0; i < roots.size() && i < 4; ++i) {
                    const data::Task* root = data::findTaskInStoreConst(store, roots[i]);
                    if (root == nullptr) {
                        continue;
                    }

                    ImVec2 min = ImGui::GetCursorScreenPos();
                    ImVec2 max = ImVec2(min.x + sideContentW, min.y + 72.0f);
                    char id[32];
                    std::snprintf(id, sizeof(id), "##rootCard%d", root->id);
                    CardVisual cv = drawCardChrome(id, min, max, 18.0f, 6.0f);

                    UrgencyColor pc = colorForPriority(root->priority);
                    ImU32 accent = urgencyToImU32(pc);
                    ImGui::GetWindowDrawList()->AddCircleFilled(
                        ImVec2(cv.drawMin.x + 18.0f, cv.drawMin.y + 24.0f), 4.0f, accent);
                    ImGui::PushTextWrapPos(cv.drawMax.x - 44.0f);
                    ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 32.0f, cv.drawMin.y + 15.0f));
                    ImGui::PushFont(fontUiSemibold());
                    ImGui::TextColored(ColTextSecondary, "%s", root->title.c_str());
                    ImGui::PopFont();

                    float completion = logic::calculateWeightedCompletion(store, root->id);
                    renderProgressRing(ImVec2(cv.drawMax.x - 26.0f, cv.drawMin.y + 27.0f), 12.0f, 3.0f,
                                       completion * 100.0f, accent);
                    ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 32.0f, cv.drawMin.y + 40.0f));
                    ImGui::TextColored(ColTextFaint, tr(K_OV_SUBTASKS_DONE_FMT),
                                       logic::countDescendants(store, root->id),
                                       completion * 100.0f);
                    ImGui::PopTextWrapPos();

                    ImGui::SetCursorScreenPos(min);
                    if (ImGui::InvisibleButton(id, ImVec2(sideContentW, 72.0f))) {
                        uiState.selectedTaskId = root->id;
                        uiState.activeNavItem = NAV_TASKS;
                    }
                    ImGui::Dummy(ImVec2(1.0f, 14.0f));
                }
            }

            ImGui::Dummy(ImVec2(1.0f, 12.0f));
            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "%s", tr(K_OV_RECENT_ACTIVITY));
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(1.0f, 8.0f));

            if (recent.empty()) {
                ImGui::TextColored(ColTextFaint, "%s", tr(K_OV_NOTHING_RECENT));
            } else {
                for (const data::Task* recentPtr : recent) {
                    const data::Task& task = *recentPtr;
                    ImVec2 min = ImGui::GetCursorScreenPos();
                    ImVec2 max = ImVec2(min.x + sideContentW, min.y + 64.0f);
                    ImGui::GetWindowDrawList()->AddLine(ImVec2(min.x, max.y),
                                                        ImVec2(max.x, max.y),
                                                        u32(ColBorder));

                    BadgeStyle badge = statusBadgeStyle(task.status);
                    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x, min.y + 6.0f),
                                                              ImVec2(min.x + 32.0f, min.y + 38.0f),
                                                              ImGui::ColorConvertFloat4ToU32(badge.bg), 10.0f);
                    ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(min.x + 16.0f, min.y + 22.0f),
                                                                4.0f, ImGui::ColorConvertFloat4ToU32(badge.dot));
                    ImGui::PushTextWrapPos(max.x - 8.0f);
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 44.0f, min.y + 6.0f));
                    ImGui::TextColored(ColTextSecondary, "%s", task.title.c_str());
                    std::string updatedAt = logic::formatDate(task.updatedAt);
                    ImGui::SetCursorScreenPos(ImVec2(min.x + 44.0f, min.y + 28.0f));
                    ImGui::TextColored(ColTextFaint, "%s",
                                       updatedAt.empty()
                                           ? tr(K_OV_UPDATED_RECENTLY)
                                           : updatedAt.c_str());
                    ImGui::PopTextWrapPos();
                    ImGui::Dummy(ImVec2(1.0f, 64.0f));
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleVar();
        }

        void renderTaskWorkspace(data::TaskStore& store, UiState& uiState) {
            float avail = ImGui::GetContentRegionAvail().x;
            float availH = ImGui::GetContentRegionAvail().y;

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 20.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            if (avail < 720.0f) {
                float treeH = 190.0f;
                float listH = (std::max)(260.0f, availH * 0.42f);

                ImGui::BeginChild("##TreePane", ImVec2(0.0f, treeH), true);
                renderTaskTreePanel(store, uiState);
                ImGui::EndChild();

                ImGui::Dummy(ImVec2(1.0f, 8.0f));
                ImGui::BeginChild("##TaskPane", ImVec2(0.0f, listH), true);
                renderTaskTablePanel(store, uiState);
                ImGui::EndChild();

                ImGui::Dummy(ImVec2(1.0f, 8.0f));
                ImGui::BeginChild("##DetailPane", ImVec2(0.0f, 0.0f), true);
                renderTaskDetailsPanel(store, uiState);
                ImGui::EndChild();

                ImGui::PopStyleVar(2);
                return;
            }

            if (avail < 1080.0f) {
                float topH = (std::max)(320.0f, availH * 0.56f);
                float treeW = (std::min)(280.0f, avail * 0.34f);
                float listW = avail - treeW - 8.0f;

                ImGui::BeginChild("##TreePane", ImVec2(treeW, topH), true);
                renderTaskTreePanel(store, uiState);
                ImGui::EndChild();

                ImGui::SameLine(0, 8.0f);
                ImGui::BeginChild("##TaskPane", ImVec2(listW, topH), true);
                renderTaskTablePanel(store, uiState);
                ImGui::EndChild();

                ImGui::Dummy(ImVec2(1.0f, 8.0f));
                ImGui::BeginChild("##DetailPane", ImVec2(0.0f, 0.0f), true);
                renderTaskDetailsPanel(store, uiState);
                ImGui::EndChild();

                ImGui::PopStyleVar(2);
                return;
            }

            float treeW = avail * 0.23f;
            float listW = avail * 0.43f;
            float detailsW = avail - treeW - listW - 16.0f;
            if (treeW < 220.0f) treeW = 220.0f;
            if (listW < 360.0f) listW = 360.0f;
            if (detailsW < 290.0f) detailsW = 290.0f;

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

            static NavItem s_prevNav    = uiState.activeNavItem;
            static double  s_switchTime = ImGui::GetTime() - 1.0;
            static int     s_slideDir   = 1;
            if (s_prevNav != uiState.activeNavItem) {
                s_slideDir   = (int)uiState.activeNavItem >= (int)s_prevNav ? 1 : -1;
                s_prevNav    = uiState.activeNavItem;
                s_switchTime = ImGui::GetTime();
            }
            double swElapsed = ImGui::GetTime() - s_switchTime;
            float  swT       = (float)(swElapsed / 0.26);
            if (swT < 0.0f) swT = 0.0f;
            if (swT > 1.0f) swT = 1.0f;
            float swEase     = 1.0f - (1.0f - swT) * (1.0f - swT) * (1.0f - swT);
            float swAlpha    = 0.25f + 0.75f * swEase;
            float swOffsetY  = (1.0f - swEase) * 32.0f * (float)s_slideDir;

            ImGui::SetNextWindowPos(ImVec2(contentX, contentY + swOffsetY));
            ImGui::SetNextWindowSize(ImVec2(contentW, contentH));
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, swAlpha);

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
                ImGui::PopStyleVar(2);
                return;
            }

            drawShellBackdrop(ImGui::GetWindowDrawList(),
                              ImGui::GetWindowPos(),
                              ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(),
                                     ImGui::GetWindowPos().y + ImGui::GetWindowHeight()));

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
                case NAV_HELP:
                    renderHelpPanel(store, uiState);
                    break;
                case NAV_HISTORY:
                    renderHistoryPanel(store, uiState);
                    break;
                case NAV_PRODUCTIVITY:
                    renderProductivityPanel(store, uiState);
                    break;
            }

            ImGui::End();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar(2);
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
