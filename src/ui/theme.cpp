#include "ui/theme.h"
#include "ui/i18n.h"
#include "logic/dates.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace ui {

    namespace {

        bool gDark = false;
        ThemeChoice gTheme = THEME_LIGHT;

        ImFont* gFontUi          = nullptr;
        ImFont* gFontUiSemibold  = nullptr;
        ImFont* gFontHeading     = nullptr;
        ImFont* gFontDisplay     = nullptr;
        ImFont* gFontMono        = nullptr;

        static const ImWchar* extendedRanges() {
            static const ImWchar ranges[] = {
                0x0020, 0x00FF,  // Latin + Latin-1 supplement
                0x0400, 0x04FF,  // Cyrillic
                0x0500, 0x052F,  // Cyrillic supplement
                0x2010, 0x205E,  // punctuation
                0,
            };
            return ranges;
        }

        ImFont* tryLoadFont(float size, const std::vector<const char*>& candidates) {
            ImGuiIO& io = ImGui::GetIO();
            for (const char* path : candidates) {
                if (std::filesystem::exists(path)) {
                    if (ImFont* font = io.Fonts->AddFontFromFileTTF(
                            path, size, nullptr, extendedRanges())) {
                        const char* segoe = "C:/Windows/Fonts/segoeui.ttf";
                        if (std::filesystem::exists(segoe)) {
                            ImFontConfig merge;
                            merge.MergeMode = true;
                            static const ImWchar cyr[] = {
                                0x0400, 0x04FF, 0x0500, 0x052F, 0,
                            };
                            io.Fonts->AddFontFromFileTTF(segoe, size, &merge, cyr);
                        }
                        return font;
                    }
                }
            }
            return nullptr;
        }

        ImFont* fallbackFont(ImFont* font) {
            if (font != nullptr) {
                return font;
            }
            ImGuiIO& io = ImGui::GetIO();
            if (!io.Fonts->Fonts.empty()) {
                return io.Fonts->Fonts[0];
            }
            return nullptr;
        }

        ImU32 accentBorderForStatus(data::Status status) {
            switch (status) {
                case data::STATUS_TODO:        return IM_COL32(148, 163, 184, 255);
                case data::STATUS_IN_PROGRESS: return IM_COL32(217, 119,   6, 255);
                case data::STATUS_DONE:        return IM_COL32(  5, 150, 105, 255);
                case data::STATUS_BLOCKED:     return IM_COL32(220,  38,  38, 255);
            }
            return IM_COL32(148, 163, 184, 255);
        }

    }

    void loadFonts() {
        ImGuiIO& io = ImGui::GetIO();

        gFontUi = tryLoadFont(15.5f, {
            "vendor/imgui/misc/fonts/Karla-Regular.ttf",
            "../vendor/imgui/misc/fonts/Karla-Regular.ttf",
            "src/vendor/imgui/misc/fonts/Karla-Regular.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        });
        gFontUiSemibold = tryLoadFont(15.5f, {
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "src/vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontHeading = tryLoadFont(21.0f, {
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "src/vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontDisplay = tryLoadFont(29.0f, {
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "src/vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontMono = tryLoadFont(14.0f, {
            "vendor/imgui/misc/fonts/Cousine-Regular.ttf",
            "../vendor/imgui/misc/fonts/Cousine-Regular.ttf",
            "src/vendor/imgui/misc/fonts/Cousine-Regular.ttf",
            "C:/Windows/Fonts/consola.ttf"
        });

        if (gFontUi != nullptr) {
            io.FontDefault = gFontUi;
        }
    }

    ImFont* fontUi() {
        return fallbackFont(gFontUi);
    }

    ImFont* fontUiSemibold() {
        return fallbackFont(gFontUiSemibold != nullptr ? gFontUiSemibold : gFontUi);
    }

    ImFont* fontHeading() {
        return fallbackFont(gFontHeading != nullptr ? gFontHeading : gFontUiSemibold);
    }

    ImFont* fontDisplay() {
        return fallbackFont(gFontDisplay != nullptr ? gFontDisplay : gFontHeading);
    }

    ImFont* fontMono() {
        return fallbackFont(gFontMono != nullptr ? gFontMono : gFontUi);
    }

    namespace {

        void setAccent(unsigned int base, unsigned int hover, unsigned int active,
                       unsigned int alt) {
            ColAccent       = HEX(base);
            ColAccentHover  = HEX(hover);
            ColAccentActive = HEX(active);
            ColAccentAlt    = HEX(alt);
        }

        void setLightPalette() {
            ColBgBase        = HEX(0xF6F8FC);
            ColBgSubtle      = HEX(0xEEF3F8);
            ColBgCard        = HEX(0xFFFFFF);
            ColBgCardSoft    = HEX(0xFBFCFE);
            ColBgActive      = HEX(0xEEE8FF);
            ColBgHover       = HEX(0xF7F4FF);
            ColBorder        = HEX(0xDCE5F0);
            ColBorderStrong  = HEX(0xC8D5E4);
            ColTextPrimary   = HEX(0x162033);
            ColTextSecondary = HEX(0x344256);
            ColTextMuted     = HEX(0x63748A);
            ColTextFaint     = HEX(0x95A2B4);
            ColShadow        = HEX(0x0F172A, 0.08f);
            ColShadowStrong  = HEX(0x0F172A, 0.14f);
            setAccent(0x7C3AED, 0x6D28D9, 0x5B21B6, 0xEC4899);
        }

        void setDarkPalette() {
            ColBgBase        = HEX(0x0F172A);
            ColBgSubtle      = HEX(0x1B2336);
            ColBgCard        = HEX(0x162033);
            ColBgCardSoft    = HEX(0x121B2C);
            ColBgActive      = HEX(0x2A2055);
            ColBgHover       = HEX(0x232B45);
            ColBorder        = HEX(0x2A3447);
            ColBorderStrong  = HEX(0x3A465D);
            ColTextPrimary   = HEX(0xE8EEF7);
            ColTextSecondary = HEX(0xC1CAD8);
            ColTextMuted     = HEX(0x8D99AC);
            ColTextFaint     = HEX(0x5F6A7E);
            ColShadow        = HEX(0x000000, 0.35f);
            ColShadowStrong  = HEX(0x000000, 0.55f);
            setAccent(0x7C3AED, 0x6D28D9, 0x5B21B6, 0xEC4899);
        }

        void setSolarizedLightPalette() {
            ColBgBase        = HEX(0xFDF6E3);
            ColBgSubtle      = HEX(0xF5EDD2);
            ColBgCard        = HEX(0xFFFCEF);
            ColBgCardSoft    = HEX(0xFAF1D7);
            ColBgActive      = HEX(0xF0E4B5);
            ColBgHover       = HEX(0xF5EAC2);
            ColBorder        = HEX(0xE6DDC2);
            ColBorderStrong  = HEX(0xC9C0A4);
            ColTextPrimary   = HEX(0x073642);
            ColTextSecondary = HEX(0x586E75);
            ColTextMuted     = HEX(0x839496);
            ColTextFaint     = HEX(0x93A1A1);
            ColShadow        = HEX(0x586E75, 0.10f);
            ColShadowStrong  = HEX(0x586E75, 0.18f);
            setAccent(0xB58900, 0x9C7600, 0x7C5E00, 0xCB4B16);
        }

        void setSolarizedDarkPalette() {
            ColBgBase        = HEX(0x002B36);
            ColBgSubtle      = HEX(0x073642);
            ColBgCard        = HEX(0x094352);
            ColBgCardSoft    = HEX(0x05303B);
            ColBgActive      = HEX(0x0E4D60);
            ColBgHover       = HEX(0x0B4254);
            ColBorder        = HEX(0x0E4D60);
            ColBorderStrong  = HEX(0x216676);
            ColTextPrimary   = HEX(0xEEE8D5);
            ColTextSecondary = HEX(0x93A1A1);
            ColTextMuted     = HEX(0x839496);
            ColTextFaint     = HEX(0x657B83);
            ColShadow        = HEX(0x000000, 0.40f);
            ColShadowStrong  = HEX(0x000000, 0.60f);
            setAccent(0x268BD2, 0x1E70AD, 0x175A8E, 0x2AA198);
        }

        void setNordPalette() {
            ColBgBase        = HEX(0x2E3440);
            ColBgSubtle      = HEX(0x3B4252);
            ColBgCard        = HEX(0x434C5E);
            ColBgCardSoft    = HEX(0x353B49);
            ColBgActive      = HEX(0x4C566A);
            ColBgHover       = HEX(0x434C5E);
            ColBorder        = HEX(0x4C566A);
            ColBorderStrong  = HEX(0x616E88);
            ColTextPrimary   = HEX(0xECEFF4);
            ColTextSecondary = HEX(0xD8DEE9);
            ColTextMuted     = HEX(0x81A1C1);
            ColTextFaint     = HEX(0x6B7589);
            ColShadow        = HEX(0x000000, 0.35f);
            ColShadowStrong  = HEX(0x000000, 0.55f);
            setAccent(0x88C0D0, 0x6FAEC0, 0x5198B0, 0xB48EAD);
        }

        void setDraculaPalette() {
            ColBgBase        = HEX(0x282A36);
            ColBgSubtle      = HEX(0x21222C);
            ColBgCard        = HEX(0x343746);
            ColBgCardSoft    = HEX(0x2B2D3A);
            ColBgActive      = HEX(0x44475A);
            ColBgHover       = HEX(0x3A3D4D);
            ColBorder        = HEX(0x44475A);
            ColBorderStrong  = HEX(0x6272A4);
            ColTextPrimary   = HEX(0xF8F8F2);
            ColTextSecondary = HEX(0xD8D8D0);
            ColTextMuted     = HEX(0x6272A4);
            ColTextFaint     = HEX(0x4F5878);
            ColShadow        = HEX(0x000000, 0.40f);
            ColShadowStrong  = HEX(0x000000, 0.60f);
            setAccent(0xBD93F9, 0xA77BEB, 0x8E5FE0, 0xFF79C6);
        }

        void setMonokaiPalette() {
            ColBgBase        = HEX(0x272822);
            ColBgSubtle      = HEX(0x1E1F1B);
            ColBgCard        = HEX(0x33342E);
            ColBgCardSoft    = HEX(0x2B2C26);
            ColBgActive      = HEX(0x49483E);
            ColBgHover       = HEX(0x3D3E36);
            ColBorder        = HEX(0x49483E);
            ColBorderStrong  = HEX(0x75715E);
            ColTextPrimary   = HEX(0xF8F8F2);
            ColTextSecondary = HEX(0xCCCCC0);
            ColTextMuted     = HEX(0x75715E);
            ColTextFaint     = HEX(0x5C594A);
            ColShadow        = HEX(0x000000, 0.40f);
            ColShadowStrong  = HEX(0x000000, 0.60f);
            setAccent(0xA6E22E, 0x8FCB1F, 0x73A816, 0xF92672);
        }

        void setHighContrastPalette() {
            ColBgBase        = HEX(0xFFFFFF);
            ColBgSubtle      = HEX(0xF0F0F0);
            ColBgCard        = HEX(0xFFFFFF);
            ColBgCardSoft    = HEX(0xFAFAFA);
            ColBgActive      = HEX(0xFFFF00);
            ColBgHover       = HEX(0xE0E0E0);
            ColBorder        = HEX(0x000000);
            ColBorderStrong  = HEX(0x000000);
            ColTextPrimary   = HEX(0x000000);
            ColTextSecondary = HEX(0x000000);
            ColTextMuted     = HEX(0x222222);
            ColTextFaint     = HEX(0x444444);
            ColShadow        = HEX(0x000000, 0.30f);
            ColShadowStrong  = HEX(0x000000, 0.55f);
            setAccent(0x0000EE, 0x0000B8, 0x000080, 0xCC0000);
        }

        bool isDarkPalette(ThemeChoice t) {
            switch (t) {
                case THEME_DARK:
                case THEME_SOLARIZED_DARK:
                case THEME_NORD:
                case THEME_DRACULA:
                case THEME_MONOKAI:
                    return true;
                default:
                    return false;
            }
        }

    }

    void applyTheme() {
        ImGuiStyle& s = ImGui::GetStyle();

        s.WindowRounding    = 0.0f;
        s.ChildRounding     = 18.0f;
        s.FrameRounding     = 10.0f;
        s.PopupRounding     = 18.0f;
        s.ScrollbarRounding = 99.0f;
        s.GrabRounding      = 99.0f;
        s.TabRounding       = 12.0f;
        s.WindowBorderSize  = 1.0f;
        s.ChildBorderSize   = 1.0f;
        s.FrameBorderSize   = 1.0f;
        s.PopupBorderSize   = 1.0f;

        s.WindowPadding           = ImVec2(22.0f, 18.0f);
        s.FramePadding            = ImVec2(12.0f, 9.0f);
        s.ItemSpacing             = ImVec2(10.0f, 8.0f);
        s.ItemInnerSpacing        = ImVec2(8.0f, 6.0f);
        s.IndentSpacing           = 18.0f;
        s.ScrollbarSize           = 8.0f;
        s.GrabMinSize             = 10.0f;
        s.SeparatorTextBorderSize = 1.0f;

        ImVec4* c = s.Colors;

        c[ImGuiCol_WindowBg]           = ColBgBase;
        c[ImGuiCol_ChildBg]            = ColBgCard;
        c[ImGuiCol_PopupBg]            = ColBgCard;
        c[ImGuiCol_Border]             = ColBorder;
        c[ImGuiCol_BorderShadow]       = ImVec4(0, 0, 0, 0);

        c[ImGuiCol_FrameBg]            = ColBgSubtle;
        c[ImGuiCol_FrameBgHovered]     = gDark ? HEX(0x232B45) : HEX(0xF3F6FB);
        c[ImGuiCol_FrameBgActive]      = ColBgActive;

        c[ImGuiCol_TitleBg]            = ColBgCard;
        c[ImGuiCol_TitleBgActive]      = ColBgCard;
        c[ImGuiCol_TitleBgCollapsed]   = ColBgSubtle;

        c[ImGuiCol_MenuBarBg]          = ColBgCard;

        c[ImGuiCol_ScrollbarBg]        = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ScrollbarGrab]        = gDark ? HEX(0x3A465D) : HEX(0xCCD6E4);
        c[ImGuiCol_ScrollbarGrabHovered] = gDark ? HEX(0x4D5B73) : HEX(0x9BA8B8);
        c[ImGuiCol_ScrollbarGrabActive]  = gDark ? HEX(0x6273A0) : HEX(0x6E7C8F);

        c[ImGuiCol_CheckMark]          = ColAccent;
        c[ImGuiCol_SliderGrab]         = ColAccent;
        c[ImGuiCol_SliderGrabActive]   = ColAccentHover;

        c[ImGuiCol_Button]             = ColAccent;
        c[ImGuiCol_ButtonHovered]      = ColAccentHover;
        c[ImGuiCol_ButtonActive]       = ColAccentActive;

        c[ImGuiCol_Header]             = ColBgActive;
        c[ImGuiCol_HeaderHovered]      = ColBgHover;
        c[ImGuiCol_HeaderActive]       = ColBgActive;

        c[ImGuiCol_Separator]          = ColBorder;
        c[ImGuiCol_SeparatorHovered]   = ColBorderStrong;
        c[ImGuiCol_SeparatorActive]    = ColAccent;

        c[ImGuiCol_ResizeGrip]         = gDark ? HEX(0x3A465D) : HEX(0xD9E2EC);
        c[ImGuiCol_ResizeGripHovered]  = withAlpha(ColAccent, 0.45f);
        c[ImGuiCol_ResizeGripActive]   = withAlpha(ColAccent, 0.80f);

        c[ImGuiCol_Tab]                = ColBgSubtle;
        c[ImGuiCol_TabHovered]         = ColBgHover;
        c[ImGuiCol_TabActive]          = ColBgCard;
        c[ImGuiCol_TabUnfocused]       = ColBgSubtle;
        c[ImGuiCol_TabUnfocusedActive] = ColBgCard;

        c[ImGuiCol_PlotLines]          = ColAccent;
        c[ImGuiCol_PlotLinesHovered]   = ColAccentAlt;
        c[ImGuiCol_PlotHistogram]      = ColAccent;
        c[ImGuiCol_PlotHistogramHovered] = ColAccentAlt;

        c[ImGuiCol_TableHeaderBg]      = ColBgSubtle;
        c[ImGuiCol_TableBorderStrong]  = ColBorder;
        c[ImGuiCol_TableBorderLight]   = gDark ? HEX(0x222B40) : HEX(0xEEF3F8);
        c[ImGuiCol_TableRowBg]         = ColBgCard;
        c[ImGuiCol_TableRowBgAlt]      = ColBgCardSoft;

        c[ImGuiCol_TextSelectedBg]     = withAlpha(ColAccent, 0.16f);
        c[ImGuiCol_DragDropTarget]     = withAlpha(ColAccent, 0.90f);
        c[ImGuiCol_NavHighlight]       = ColAccent;
        c[ImGuiCol_NavWindowingHighlight] = withAlpha(ColAccent, 0.7f);
        c[ImGuiCol_NavWindowingDimBg]  = HEX(0x0F172A, 0.2f);
        c[ImGuiCol_ModalWindowDimBg]   = HEX(0x0F172A, 0.35f);

        c[ImGuiCol_Text]               = ColTextPrimary;
        c[ImGuiCol_TextDisabled]       = ColTextFaint;
    }

    void applyDarkTheme() {
        applyThemeChoice(THEME_DARK);
    }

    void applyLightTheme() {
        applyThemeChoice(THEME_LIGHT);
    }

    void applyThemeChoice(ThemeChoice t) {
        if (t < 0 || t >= THEME_COUNT) {
            t = THEME_LIGHT;
        }
        gTheme = t;
        gDark  = isDarkPalette(t);
        switch (t) {
            case THEME_LIGHT:           setLightPalette();          break;
            case THEME_DARK:            setDarkPalette();           break;
            case THEME_SOLARIZED_LIGHT: setSolarizedLightPalette(); break;
            case THEME_SOLARIZED_DARK:  setSolarizedDarkPalette();  break;
            case THEME_NORD:            setNordPalette();           break;
            case THEME_DRACULA:         setDraculaPalette();        break;
            case THEME_MONOKAI:         setMonokaiPalette();        break;
            case THEME_HIGH_CONTRAST:   setHighContrastPalette();   break;
            default:                    setLightPalette();          break;
        }
        applyTheme();
    }

    ThemeChoice currentThemeChoice() {
        return gTheme;
    }

    const char* themeName(ThemeChoice t) {
        switch (t) {
            case THEME_LIGHT:           return "Light";
            case THEME_DARK:            return "Dark";
            case THEME_SOLARIZED_LIGHT: return "Solarized Light";
            case THEME_SOLARIZED_DARK:  return "Solarized Dark";
            case THEME_NORD:            return "Nord";
            case THEME_DRACULA:         return "Dracula";
            case THEME_MONOKAI:         return "Monokai";
            case THEME_HIGH_CONTRAST:   return "High Contrast";
            default:                    return "?";
        }
    }

    bool isDarkTheme() {
        return gDark;
    }

    ImU32 cardBgU32() {
        return ImGui::ColorConvertFloat4ToU32(ColBgCard);
    }

    ImU32 cardBorderU32() {
        return ImGui::ColorConvertFloat4ToU32(ColBorder);
    }

    UrgencyColor urgencyForDeadline(const data::Date& deadline,
                                    const data::Date& today,
                                    data::Status status) {
        if (logic::isDateZero(deadline))
            return UrgencyColor{ 0.58f, 0.71f, 0.87f, 1.0f };
        if (status == data::STATUS_DONE)
            return UrgencyColor{ 0.45f, 0.54f, 0.65f, 1.0f };
        int days = logic::daysBetween(today, deadline);
        if (days < 0)
            return UrgencyColor{ 0.863f, 0.149f, 0.149f, 1.0f };
        if (days <= 3)
            return UrgencyColor{ 0.851f, 0.467f, 0.047f, 1.0f };
        if (days <= 14)
            return UrgencyColor{ 0.145f, 0.388f, 0.922f, 1.0f };
        return UrgencyColor{ 0.024f, 0.588f, 0.412f, 1.0f };
    }

    UrgencyColor colorForPriority(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:      return UrgencyColor{ 0.024f, 0.588f, 0.412f, 1.0f };
            case data::PRIORITY_MEDIUM:   return UrgencyColor{ 0.145f, 0.388f, 0.922f, 1.0f };
            case data::PRIORITY_HIGH:     return UrgencyColor{ 0.918f, 0.345f, 0.047f, 1.0f };
            case data::PRIORITY_CRITICAL: return UrgencyColor{ 0.863f, 0.149f, 0.149f, 1.0f };
        }
        return UrgencyColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    }

    const char* priorityLabel(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:      return tr(K_PRI_LOW);
            case data::PRIORITY_MEDIUM:   return tr(K_PRI_MEDIUM);
            case data::PRIORITY_HIGH:     return tr(K_PRI_HIGH);
            case data::PRIORITY_CRITICAL: return tr(K_PRI_CRITICAL);
        }
        return "?";
    }

    const char* statusLabel(data::Status s) {
        switch (s) {
            case data::STATUS_TODO:        return tr(K_ST_TODO);
            case data::STATUS_IN_PROGRESS: return tr(K_ST_IN_PROGRESS);
            case data::STATUS_DONE:        return tr(K_ST_DONE);
            case data::STATUS_BLOCKED:     return tr(K_ST_BLOCKED);
        }
        return "?";
    }

    BadgeStyle statusBadgeStyle(data::Status s) {
        switch (s) {
            case data::STATUS_TODO:
                return { tr(K_ST_TODO),        HEX(0xF1F5F9), HEX(0x64748B), HEX(0x94A3B8) };
            case data::STATUS_IN_PROGRESS:
                return { tr(K_ST_IN_PROGRESS), HEX(0xFFFBEB), HEX(0xB45309), HEX(0xD97706) };
            case data::STATUS_DONE:
                return { tr(K_ST_DONE),        HEX(0xECFDF5), HEX(0x065F46), HEX(0x059669) };
            case data::STATUS_BLOCKED:
                return { tr(K_ST_BLOCKED),     HEX(0xFEF2F2), HEX(0x991B1B), HEX(0xDC2626) };
        }
        return { "?", HEX(0xF1F5F9), HEX(0x64748B), HEX(0x94A3B8) };
    }

    BadgeStyle priorityBadgeStyle(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:
                return { tr(K_PRI_LOW),      HEX(0xECFDF5), HEX(0x059669), HEX(0x059669) };
            case data::PRIORITY_MEDIUM:
                return { tr(K_PRI_MEDIUM),   HEX(0xEFF6FF), HEX(0x2563EB), HEX(0x2563EB) };
            case data::PRIORITY_HIGH:
                return { tr(K_PRI_HIGH),     HEX(0xFFF7ED), HEX(0xEA580C), HEX(0xEA580C) };
            case data::PRIORITY_CRITICAL:
                return { tr(K_PRI_CRITICAL), HEX(0xFEF2F2), HEX(0xDC2626), HEX(0xDC2626) };
        }
        return { "?", HEX(0xF1F5F9), HEX(0x64748B), HEX(0x94A3B8) };
    }

    void renderBadge(const char* label, ImVec4 bgCol, ImVec4 textCol,
                     bool hasDot, ImVec4 dotCol) {
        ImDrawList* dl  = ImGui::GetWindowDrawList();
        ImVec2 pos      = ImGui::GetCursorScreenPos();
        float padX      = 10.0f;
        float padY      = 4.0f;
        ImFont* font    = fontUiSemibold();
        float fontSize  = font != nullptr ? font->LegacySize : ImGui::GetFontSize();
        ImVec2 textSz   = ImGui::CalcTextSize(label);
        float dotW      = hasDot ? 14.0f : 0.0f;
        float totalW    = textSz.x + dotW + padX * 2.0f;
        float totalH    = textSz.y + padY * 2.0f;

        dl->AddRectFilled(pos, ImVec2(pos.x + totalW, pos.y + totalH),
                          ImGui::ColorConvertFloat4ToU32(bgCol), 99.0f);

        if (hasDot) {
            float cx = pos.x + padX + 3.0f;
            float cy = pos.y + totalH * 0.5f;
            dl->AddCircleFilled(ImVec2(cx, cy), 2.8f,
                                ImGui::ColorConvertFloat4ToU32(dotCol));
        }

        dl->AddText(font, fontSize,
                    ImVec2(pos.x + padX + dotW, pos.y + padY),
                    ImGui::ColorConvertFloat4ToU32(textCol), label);

        ImGui::SetCursorScreenPos(pos);
        ImGui::Dummy(ImVec2(totalW, totalH));
    }

    void drawSoftShadow(ImDrawList* dl, ImVec2 min, ImVec2 max,
                        float rounding, ImU32 color, ImVec2 offset) {
        if (dl == nullptr) {
            return;
        }
        dl->AddRectFilled(ImVec2(min.x + offset.x, min.y + offset.y),
                          ImVec2(max.x + offset.x, max.y + offset.y),
                          color, rounding);
        dl->AddRectFilled(ImVec2(min.x + offset.x * 0.5f, min.y + offset.y * 0.5f),
                          ImVec2(max.x + offset.x * 0.5f, max.y + offset.y * 0.5f),
                          IM_COL32(15, 23, 42, 8), rounding);
    }

    void drawGradientRect(ImDrawList* dl, ImVec2 min, ImVec2 max,
                          float rounding, ImU32 left, ImU32 right) {
        if (dl == nullptr) {
            return;
        }
        int start = dl->VtxBuffer.Size;
        dl->AddRectFilled(min, max, IM_COL32_WHITE, rounding);
        int end = dl->VtxBuffer.Size;
        ImGui::ShadeVertsLinearColorGradientKeepAlpha(dl, start, end, min,
                                                      ImVec2(max.x, min.y),
                                                      left, right);
    }

    bool gradientButton(const char* id, const char* label,
                        ImVec2 size, float rounding) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        if (size.x <= 0.0f) {
            size.x = ImGui::CalcTextSize(label).x + 28.0f;
        }
        if (size.y <= 0.0f) {
            size.y = 38.0f;
        }
        ImVec2 end = ImVec2(pos.x + size.x, pos.y + size.y);

        ImGui::InvisibleButton(id, size);
        bool hovered = ImGui::IsItemHovered();
        bool held    = ImGui::IsItemActive();
        bool pressed = ImGui::IsItemClicked();

        ImDrawList* dl = ImGui::GetWindowDrawList();
        drawSoftShadow(dl, pos, end, rounding,
                       hovered ? IM_COL32(124, 58, 237, 54)
                               : IM_COL32(124, 58, 237, 36),
                       ImVec2(0.0f, hovered ? 7.0f : 6.0f));
        drawGradientRect(dl, pos, end, rounding, GradLeft, GradRight);

        if (held) {
            dl->AddRectFilled(pos, end, IM_COL32(15, 23, 42, 26), rounding);
        } else if (hovered) {
            dl->AddRectFilled(pos, end, IM_COL32(255, 255, 255, 22), rounding);
        }

        ImVec2 textSz = ImGui::CalcTextSize(label);
        ImFont* font  = fontUiSemibold();
        float fontSz  = font != nullptr ? font->LegacySize : ImGui::GetFontSize();
        dl->AddText(font, fontSz,
                    ImVec2(pos.x + (size.x - textSz.x) * 0.5f,
                           pos.y + (size.y - textSz.y) * 0.5f - 1.0f),
                    IM_COL32(255, 255, 255, 255), label);
        return pressed;
    }

    void renderProgressRing(ImVec2 centre, float radius, float thickness,
                            float pct, ImU32 fgCol, ImU32 bgCol) {
        ImDrawList* dl   = ImGui::GetWindowDrawList();
        const int segs   = 64;
        const float kPi  = 3.14159265358979323846f;
        const float pi2  = 2.0f * kPi;

        dl->PathArcTo(centre, radius, 0, pi2, segs);
        dl->PathStroke(bgCol, false, thickness);

        if (pct > 0.001f) {
            float startAngle = -kPi * 0.5f;
            float endAngle   = startAngle + pi2 * (pct / 100.0f);
            dl->PathArcTo(centre, radius, startAngle, endAngle, segs);
            dl->PathStroke(pct >= 99.5f ? accentBorderForStatus(data::STATUS_DONE)
                                        : fgCol,
                           false, thickness);
        }
    }

}
