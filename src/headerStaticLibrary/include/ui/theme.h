#pragma once

#include "libraries.h"
#include "data/types.h"
#include "imgui.h"

namespace ui {

    static inline ImVec4 HEX(unsigned int hex, float a = 1.0f) {
        return ImVec4(
            ((hex >> 16) & 0xFF) / 255.0f,
            ((hex >>  8) & 0xFF) / 255.0f,
            ( hex        & 0xFF) / 255.0f,
            a);
    }

    inline ImVec4 ColBgBase        = HEX(0xF6F8FC);
    inline ImVec4 ColBgSubtle      = HEX(0xEEF3F8);
    inline ImVec4 ColBgCard        = HEX(0xFFFFFF);
    inline ImVec4 ColBgCardSoft    = HEX(0xFBFCFE);
    inline ImVec4 ColBgActive      = HEX(0xEEE8FF);
    inline ImVec4 ColBgHover       = HEX(0xF7F4FF);
    inline ImVec4 ColBorder        = HEX(0xDCE5F0);
    inline ImVec4 ColBorderStrong  = HEX(0xC8D5E4);
    inline ImVec4 ColTextPrimary   = HEX(0x162033);
    inline ImVec4 ColTextSecondary = HEX(0x344256);
    inline ImVec4 ColTextMuted     = HEX(0x63748A);
    inline ImVec4 ColTextFaint     = HEX(0x95A2B4);
    inline ImVec4 ColShadow        = HEX(0x0F172A, 0.08f);
    inline ImVec4 ColShadowStrong  = HEX(0x0F172A, 0.14f);

    inline const ImU32 GradLeft  = IM_COL32(124,  58, 237, 255);
    inline const ImU32 GradMid   = IM_COL32(236,  72, 153, 255);
    inline const ImU32 GradRight = IM_COL32(249, 115,  22, 255);

    void loadFonts();
    void applyTheme();
    void applyDarkTheme();
    void applyLightTheme();
    bool isDarkTheme();
    ImU32 cardBgU32();
    ImU32 cardBorderU32();

    ImFont* fontUi();
    ImFont* fontUiSemibold();
    ImFont* fontHeading();
    ImFont* fontDisplay();
    ImFont* fontMono();

    struct UrgencyColor { float r, g, b, a; };

    UrgencyColor urgencyForDeadline(const data::Date& deadline,
                                    const data::Date& today,
                                    data::Status status);
    UrgencyColor colorForPriority(data::Priority p);

    const char* priorityLabel(data::Priority p);
    const char* statusLabel(data::Status s);

    struct BadgeStyle {
        const char* label;
        ImVec4 bg;
        ImVec4 text;
        ImVec4 dot;
    };

    BadgeStyle statusBadgeStyle(data::Status s);
    BadgeStyle priorityBadgeStyle(data::Priority p);

    void renderBadge(const char* label, ImVec4 bgCol, ImVec4 textCol,
                     bool hasDot = false, ImVec4 dotCol = ImVec4(0, 0, 0, 0));

    void drawSoftShadow(ImDrawList* dl, ImVec2 min, ImVec2 max,
                        float rounding, ImU32 color = IM_COL32(15, 23, 42, 16),
                        ImVec2 offset = ImVec2(0.0f, 6.0f));
    void drawGradientRect(ImDrawList* dl, ImVec2 min, ImVec2 max,
                          float rounding = 0.0f,
                          ImU32 left = GradLeft,
                          ImU32 right = GradRight);
    bool gradientButton(const char* id, const char* label,
                        ImVec2 size, float rounding = 14.0f);

    void renderProgressRing(ImVec2 centre, float radius, float thickness,
                            float pct, ImU32 fgCol,
                            ImU32 bgCol = IM_COL32(226, 232, 240, 255));

}
