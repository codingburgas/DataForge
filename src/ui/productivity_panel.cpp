#include "ui/productivity_panel.h"
#include "ui/theme.h"
#include "logic/productivity.h"
#include "logic/dates.h"
#include "data/date.h"
#include "imgui.h"

namespace ui {

    namespace {

        void renderStatChip(const char* label, const char* value, ImU32 accent, float width) {
            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + width, min.y + 96.0f);
            char id[64];
            std::snprintf(id, sizeof(id), "##prodChip_%s", label);
            CardVisual cv = drawCardChrome(id, min, max, 18.0f, 5.0f);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddCircleFilled(ImVec2(cv.drawMin.x + 28.0f, cv.drawMin.y + 36.0f), 6.0f, accent);

            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 56.0f, cv.drawMin.y + 14.0f));
            ImGui::PushFont(fontDisplay());
            ImGui::TextColored(ColTextPrimary, "%s", value);
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(cv.drawMin.x + 56.0f, cv.drawMin.y + 56.0f));
            ImGui::TextColored(ColTextMuted, "%s", label);

            ImGui::SetCursorScreenPos(min);
            ImGui::Dummy(ImVec2(width, 96.0f));
        }

        void renderPieChart(const char* title,
                            const float* vals, const ImU32* cols,
                            const char* const* names, int count,
                            float width) {
            float total = 0.0f;
            for (int i = 0; i < count; ++i) {
                total += vals[i];
            }

            ImVec2 origin = ImGui::GetCursorScreenPos();
            float cardH = 220.0f;
            ImVec2 cardMinHit = origin;
            ImVec2 cardMaxHit = ImVec2(origin.x + width, origin.y + cardH);
            char pid[96];
            std::snprintf(pid, sizeof(pid), "##prodPie_%s", title);
            CardVisual pieCv = drawCardChrome(pid, cardMinHit, cardMaxHit, 18.0f, 5.0f);
            ImVec2 cardMin = pieCv.drawMin;
            ImVec2 cardMax = pieCv.drawMax;
            ImDrawList* dl = ImGui::GetWindowDrawList();

            ImGui::SetCursorScreenPos(ImVec2(cardMin.x + 16.0f, cardMin.y + 14.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "%s", title);
            ImGui::PopFont();

            float radius = 70.0f;
            ImVec2 centre = ImVec2(cardMin.x + 16.0f + radius + 8.0f,
                                   cardMin.y + 14.0f + 24.0f + radius);

            if (total <= 0.0f) {
                dl->AddCircleFilled(centre, radius,
                                    ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 64);
                dl->AddCircle(centre, radius, cardBorderU32(), 64, 1.5f);
                ImGui::SetCursorScreenPos(ImVec2(centre.x - 28.0f, centre.y - 8.0f));
                ImGui::TextColored(ColTextFaint, "No data");
            } else {
                const float TWO_PI = 6.2831853f;
                float startAngle = -1.5707963f;
                for (int i = 0; i < count; ++i) {
                    if (vals[i] <= 0.0f) {
                        continue;
                    }
                    float frac = vals[i] / total;
                    float endAngle = startAngle + frac * TWO_PI;
                    int segs = (int)(frac * 64.0f) + 8;

                    if (frac >= 0.999f) {
                        dl->AddCircleFilled(centre, radius, cols[i], 64);
                    } else {
                        dl->PathClear();
                        dl->PathLineTo(centre);
                        dl->PathArcTo(centre, radius, startAngle, endAngle, segs);
                        dl->PathFillConvex(cols[i]);
                    }
                    startAngle = endAngle;
                }
                dl->AddCircleFilled(centre, radius * 0.45f, cardBgU32(), 48);
                dl->AddCircle(centre, radius * 0.45f, cardBorderU32(), 48, 1.0f);

                char totalBuf[16];
                std::snprintf(totalBuf, sizeof(totalBuf), "%d", (int)total);
                ImVec2 sz = ImGui::CalcTextSize(totalBuf);
                ImGui::SetCursorScreenPos(ImVec2(centre.x - sz.x * 0.5f,
                                                 centre.y - sz.y - 2.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%s", totalBuf);
                ImGui::PopFont();
                ImVec2 sz2 = ImGui::CalcTextSize("total");
                ImGui::SetCursorScreenPos(ImVec2(centre.x - sz2.x * 0.5f,
                                                 centre.y + 2.0f));
                ImGui::TextColored(ColTextFaint, "total");
            }

            float legendX = cardMin.x + 16.0f + radius * 2.0f + 32.0f;
            float legendY = cardMin.y + 42.0f;
            float rowH = 28.0f;
            for (int i = 0; i < count; ++i) {
                float pct = (total > 0.0f) ? (vals[i] / total * 100.0f) : 0.0f;
                ImVec2 sw = ImVec2(legendX, legendY + i * rowH);
                dl->AddRectFilled(sw, ImVec2(sw.x + 14.0f, sw.y + 14.0f),
                                  cols[i], 4.0f);
                ImGui::SetCursorScreenPos(ImVec2(sw.x + 22.0f, sw.y - 2.0f));
                ImGui::PushFont(fontUiSemibold());
                ImGui::TextColored(ColTextPrimary, "%s", names[i]);
                ImGui::PopFont();
                ImGui::SetCursorScreenPos(ImVec2(sw.x + 22.0f, sw.y + 14.0f));
                ImGui::TextColored(ColTextFaint, "%d  %.1f%%",
                                   (int)vals[i], pct);
            }

            ImGui::SetCursorScreenPos(origin);
            ImGui::Dummy(ImVec2(width, cardH));
        }

        void renderXpBar(const data::ProductivityState& p, float width) {
            int level = logic::currentLevel(p);
            float pct = logic::levelProgress(p);
            int into  = p.xp % logic::XP_PER_LEVEL;

            ImGui::PushFont(fontHeading());
            ImGui::TextColored(ColTextPrimary, "Level %d", level);
            ImGui::PopFont();
            ImGui::SameLine(0, 14.0f);
            ImGui::TextColored(ColTextFaint, "%d / %d XP",
                               into, logic::XP_PER_LEVEL);

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + 12.0f),
                              ImGui::ColorConvertFloat4ToU32(ColBgSubtle), 99.0f);
            dl->AddRectFilled(pos,
                              ImVec2(pos.x + width * pct, pos.y + 12.0f),
                              GradLeft, 99.0f);
            ImGui::Dummy(ImVec2(width, 12.0f));
        }

    }

    void renderProductivityPanel(const data::TaskStore& store, UiState& /*uiState*/) {
        const data::ProductivityState& p = store.productivity;

        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Productivity Dashboard");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted,
                           "XP, badges, and streaks across this store.");
        ImGui::Dummy(ImVec2(1.0f, 14.0f));

        float avail = ImGui::GetContentRegionAvail().x;
        renderXpBar(p, avail);
        ImGui::Dummy(ImVec2(1.0f, 18.0f));

        std::string todayIso = data::formatDate(logic::today());
        int streak = logic::currentStreak(p, todayIso);

        char xpBuf[32];
        std::snprintf(xpBuf, sizeof(xpBuf), "%d", p.xp);
        char doneBuf[32];
        std::snprintf(doneBuf, sizeof(doneBuf), "%d", p.totalDone);
        char createdBuf[32];
        std::snprintf(createdBuf, sizeof(createdBuf), "%d", p.totalCreated);
        char streakBuf[32];
        std::snprintf(streakBuf, sizeof(streakBuf), "%d", streak);

        float gap = 14.0f;
        float chipW = (avail - gap * 3.0f) * 0.25f;
        renderStatChip("Total XP", xpBuf, IM_COL32(124, 58, 237, 255), chipW);
        ImGui::SameLine(0, gap);
        renderStatChip("Tasks Done", doneBuf, IM_COL32(5, 150, 105, 255), chipW);
        ImGui::SameLine(0, gap);
        renderStatChip("Tasks Created", createdBuf, IM_COL32(37, 99, 235, 255), chipW);
        ImGui::SameLine(0, gap);
        renderStatChip("Day Streak", streakBuf, IM_COL32(234, 88, 12, 255), chipW);

        ImGui::Dummy(ImVec2(1.0f, 22.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Badges");
        ImGui::PopFont();
        ImGui::Dummy(ImVec2(1.0f, 8.0f));

        std::vector<std::string> all = logic::allBadgeIds();
        float badgeGap = 12.0f;
        int columns = 3;
        float badgeW = (avail - badgeGap * (columns - 1)) / columns;

        for (std::size_t i = 0; i < all.size(); ++i) {
            const std::string& id = all[i];
            bool earned = logic::hasBadge(p, id);

            ImVec2 min = ImGui::GetCursorScreenPos();
            ImVec2 max = ImVec2(min.x + badgeW, min.y + 86.0f);
            char bid[96];
            std::snprintf(bid, sizeof(bid), "##prodBadge_%s", id.c_str());
            bool hovered = ImGui::IsMouseHoveringRect(min, max);
            float hoverT = animateHover(bid, hovered, 0.18f);
            float lift   = hoverT * 5.0f;
            ImVec2 dMin  = ImVec2(min.x, min.y - lift);
            ImVec2 dMax  = ImVec2(max.x, max.y - lift);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            drawSoftShadow(dl, dMin, dMax, 16.0f,
                           IM_COL32(15, 23, 42, 14 + (int)(50.0f * hoverT)),
                           ImVec2(0.0f, 5.0f + 10.0f * hoverT));
            dl->AddRectFilled(dMin, dMax,
                              ImGui::ColorConvertFloat4ToU32(
                                  earned ? ColBgActive : ColBgCardSoft),
                              16.0f);
            dl->AddRect(dMin, dMax,
                        earned ? ImGui::ColorConvertFloat4ToU32(ColAccent)
                               : (hovered ? ImGui::ColorConvertFloat4ToU32(ColBorderStrong)
                                          : cardBorderU32()),
                        16.0f, 0, earned ? 1.6f : 1.0f);
            dl->AddLine(ImVec2(dMin.x + 8.0f, dMin.y + 1.0f),
                        ImVec2(dMax.x - 8.0f, dMin.y + 1.0f),
                        IM_COL32(255, 255, 255, (int)(60 + 90 * hoverT)), 1.0f);

            dl->AddCircleFilled(ImVec2(dMin.x + 26.0f, dMin.y + 30.0f), 12.0f,
                                earned ? ImGui::ColorConvertFloat4ToU32(ColAccent)
                                       : ImGui::ColorConvertFloat4ToU32(ColBorderStrong));

            ImGui::SetCursorScreenPos(ImVec2(dMin.x + 50.0f, dMin.y + 14.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(earned ? ColTextPrimary : ColTextMuted,
                               "%s", logic::badgeLabel(id));
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(dMin.x + 50.0f, dMin.y + 38.0f));
            ImGui::PushTextWrapPos(dMax.x - 12.0f);
            ImGui::TextColored(ColTextFaint, "%s", logic::badgeDescription(id));
            ImGui::PopTextWrapPos();
            ImGui::SetCursorScreenPos(ImVec2(dMin.x + 50.0f, dMax.y - 22.0f));
            ImGui::TextColored(earned ? HEX(0x059669) : ColTextFaint,
                               earned ? "Earned" : "Locked");

            ImGui::SetCursorScreenPos(min);
            ImGui::Dummy(ImVec2(badgeW, 86.0f));

            if ((i + 1) % columns != 0 && i + 1 < all.size()) {
                ImGui::SameLine(0, badgeGap);
            } else {
                ImGui::Dummy(ImVec2(1.0f, badgeGap));
            }
        }

        ImGui::Dummy(ImVec2(1.0f, 22.0f));
        ImGui::PushFont(fontHeading());
        ImGui::TextColored(ColTextPrimary, "Task Mix");
        ImGui::PopFont();
        ImGui::TextColored(ColTextMuted,
                           "Distribution by status and priority across all tasks.");
        ImGui::Dummy(ImVec2(1.0f, 10.0f));

        float byStatus[4] = {};
        float byPriority[4] = {};
        for (const data::Task& t : store.tasks) {
            int s = static_cast<int>(t.status);
            int pr = static_cast<int>(t.priority);
            if (s >= 0 && s < 4) byStatus[s] += 1.0f;
            if (pr >= 0 && pr < 4) byPriority[pr] += 1.0f;
        }

        ImU32 statusCols[4] = {
            IM_COL32(148, 163, 184, 255),
            IM_COL32(217, 119,   6, 255),
            IM_COL32(  5, 150, 105, 255),
            IM_COL32(220,  38,  38, 255),
        };
        const char* statusNames[4] = {
            statusLabel(data::STATUS_TODO),
            statusLabel(data::STATUS_IN_PROGRESS),
            statusLabel(data::STATUS_DONE),
            statusLabel(data::STATUS_BLOCKED),
        };

        ImU32 priorityCols[4] = {
            IM_COL32(  5, 150, 105, 255),
            IM_COL32( 37,  99, 235, 255),
            IM_COL32(234,  88,  12, 255),
            IM_COL32(220,  38,  38, 255),
        };
        const char* priorityNames[4] = {
            priorityLabel(data::PRIORITY_LOW),
            priorityLabel(data::PRIORITY_MEDIUM),
            priorityLabel(data::PRIORITY_HIGH),
            priorityLabel(data::PRIORITY_CRITICAL),
        };

        float pieGap = 14.0f;
        float pieW = (avail - pieGap) * 0.5f;
        renderPieChart("By Status", byStatus, statusCols, statusNames, 4, pieW);
        ImGui::SameLine(0, pieGap);
        renderPieChart("By Priority", byPriority, priorityCols, priorityNames, 4, pieW);
    }

}
