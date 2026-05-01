#include "ui/theme.h"
#include "logic/dates.h"
#include "imgui.h"

namespace ui {

    void applyDarkTheme() {
        ImGui::StyleColorsDark();
        ImGuiStyle& s     = ImGui::GetStyle();
        s.WindowRounding  = 4.0f;
        s.FrameRounding   = 3.0f;
        s.ScrollbarRounding = 3.0f;
        s.GrabRounding    = 3.0f;
        s.TabRounding     = 3.0f;
        s.PopupRounding   = 3.0f;
        s.WindowPadding   = ImVec2(10.0f, 10.0f);
        s.FramePadding    = ImVec2(6.0f, 4.0f);
        s.ItemSpacing     = ImVec2(8.0f, 6.0f);
        s.IndentSpacing   = 18.0f;
    }

    void applyLightTheme() {
        ImGui::StyleColorsLight();
        ImGuiStyle& s     = ImGui::GetStyle();
        s.WindowRounding  = 4.0f;
        s.FrameRounding   = 3.0f;
        s.ScrollbarRounding = 3.0f;
        s.GrabRounding    = 3.0f;
        s.TabRounding     = 3.0f;
        s.PopupRounding   = 3.0f;
        s.WindowPadding   = ImVec2(10.0f, 10.0f);
        s.FramePadding    = ImVec2(6.0f, 4.0f);
        s.ItemSpacing     = ImVec2(8.0f, 6.0f);
        s.IndentSpacing   = 18.0f;
    }

    UrgencyColor urgencyForDeadline(const data::Date& deadline,
                                    const data::Date& today,
                                    data::Status status) {
        UrgencyColor c{ 0.75f, 0.75f, 0.75f, 1.0f };
        if (logic::isDateZero(deadline)) {
            return c;
        }
        if (status == data::STATUS_DONE) {
            c = UrgencyColor{ 0.55f, 0.55f, 0.55f, 1.0f };
            return c;
        }
        int days = logic::daysBetween(today, deadline);
        if (days < 0) {
            c = UrgencyColor{ 0.95f, 0.35f, 0.35f, 1.0f };
        } else if (days <= 3) {
            c = UrgencyColor{ 0.98f, 0.75f, 0.25f, 1.0f };
        } else if (days <= 14) {
            c = UrgencyColor{ 0.85f, 0.85f, 0.55f, 1.0f };
        } else {
            c = UrgencyColor{ 0.45f, 0.85f, 0.55f, 1.0f };
        }
        return c;
    }

    UrgencyColor colorForPriority(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:      return UrgencyColor{ 0.55f, 0.75f, 0.95f, 1.0f };
            case data::PRIORITY_MEDIUM:   return UrgencyColor{ 0.85f, 0.85f, 0.55f, 1.0f };
            case data::PRIORITY_HIGH:     return UrgencyColor{ 0.98f, 0.70f, 0.30f, 1.0f };
            case data::PRIORITY_CRITICAL: return UrgencyColor{ 0.97f, 0.35f, 0.35f, 1.0f };
        }
        return UrgencyColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    }

    const char* priorityLabel(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:      return "Low";
            case data::PRIORITY_MEDIUM:   return "Medium";
            case data::PRIORITY_HIGH:     return "High";
            case data::PRIORITY_CRITICAL: return "Critical";
        }
        return "?";
    }

    const char* statusLabel(data::Status s) {
        switch (s) {
            case data::STATUS_TODO:        return "To-do";
            case data::STATUS_IN_PROGRESS: return "In progress";
            case data::STATUS_DONE:        return "Done";
            case data::STATUS_BLOCKED:     return "Blocked";
        }
        return "?";
    }

}
