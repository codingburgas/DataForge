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
            ImDrawList* dl = ImGui::GetWindowDrawList();
            drawSoftShadow(dl, min, max, 18.0f);
            dl->AddRectFilled(min, max, cardBgU32(), 18.0f);
            dl->AddRect(min, max, cardBorderU32(), 18.0f);
            dl->AddCircleFilled(ImVec2(min.x + 28.0f, min.y + 36.0f), 6.0f, accent);

            ImGui::SetCursorScreenPos(ImVec2(min.x + 56.0f, min.y + 14.0f));
            ImGui::PushFont(fontDisplay());
            ImGui::TextColored(ColTextPrimary, "%s", value);
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(min.x + 56.0f, min.y + 56.0f));
            ImGui::TextColored(ColTextMuted, "%s", label);

            ImGui::SetCursorScreenPos(min);
            ImGui::Dummy(ImVec2(width, 96.0f));
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
                              IM_COL32(241, 245, 249, 255), 99.0f);
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
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(min, max,
                              earned ? IM_COL32(238, 233, 255, 255)
                                     : IM_COL32(248, 250, 253, 255),
                              16.0f);
            dl->AddRect(min, max,
                        earned ? IM_COL32(124, 58, 237, 255)
                               : cardBorderU32(),
                        16.0f, 0, earned ? 1.6f : 1.0f);

            dl->AddCircleFilled(ImVec2(min.x + 26.0f, min.y + 30.0f), 12.0f,
                                earned ? IM_COL32(124, 58, 237, 255)
                                       : IM_COL32(203, 213, 225, 255));

            ImGui::SetCursorScreenPos(ImVec2(min.x + 50.0f, min.y + 14.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(earned ? ColTextPrimary : ColTextMuted,
                               "%s", logic::badgeLabel(id));
            ImGui::PopFont();
            ImGui::SetCursorScreenPos(ImVec2(min.x + 50.0f, min.y + 38.0f));
            ImGui::PushTextWrapPos(max.x - 12.0f);
            ImGui::TextColored(ColTextFaint, "%s", logic::badgeDescription(id));
            ImGui::PopTextWrapPos();
            ImGui::SetCursorScreenPos(ImVec2(min.x + 50.0f, max.y - 22.0f));
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
    }

}
