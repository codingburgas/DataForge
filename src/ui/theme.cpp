#include "ui/theme.h"
#include "logic/dates.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace ui {

    namespace {

        ImFont* gFontUi          = nullptr;
        ImFont* gFontUiSemibold  = nullptr;
        ImFont* gFontHeading     = nullptr;
        ImFont* gFontDisplay     = nullptr;
        ImFont* gFontMono        = nullptr;

        ImFont* tryLoadFont(float size, const std::vector<const char*>& candidates) {
            ImGuiIO& io = ImGui::GetIO();
            for (std::size_t i = 0; i < candidates.size(); ++i) {
                const char* path = candidates[i];
                if (std::filesystem::exists(path)) {
                    if (ImFont* font = io.Fonts->AddFontFromFileTTF(path, size)) {
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
            "../vendor/imgui/misc/fonts/Karla-Regular.ttf",
            "vendor/imgui/misc/fonts/Karla-Regular.ttf",
            "C:/Windows/Fonts/segoeui.ttf"
        });
        gFontUiSemibold = tryLoadFont(15.5f, {
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontHeading = tryLoadFont(21.0f, {
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontDisplay = tryLoadFont(29.0f, {
            "../vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "vendor/imgui/misc/fonts/Roboto-Medium.ttf",
            "C:/Windows/Fonts/seguisb.ttf",
            "C:/Windows/Fonts/calibrib.ttf"
        });
        gFontMono = tryLoadFont(14.0f, {
            "../vendor/imgui/misc/fonts/Cousine-Regular.ttf",
            "vendor/imgui/misc/fonts/Cousine-Regular.ttf",
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
        c[ImGuiCol_FrameBgHovered]     = HEX(0xF3F6FB);
        c[ImGuiCol_FrameBgActive]      = ColBgActive;

        c[ImGuiCol_TitleBg]            = ColBgCard;
        c[ImGuiCol_TitleBgActive]      = ColBgCard;
        c[ImGuiCol_TitleBgCollapsed]   = ColBgSubtle;

        c[ImGuiCol_MenuBarBg]          = ColBgCard;

        c[ImGuiCol_ScrollbarBg]        = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ScrollbarGrab]      = HEX(0xCCD6E4);
        c[ImGuiCol_ScrollbarGrabHovered] = HEX(0x9BA8B8);
        c[ImGuiCol_ScrollbarGrabActive]  = HEX(0x6E7C8F);

        c[ImGuiCol_CheckMark]          = HEX(0x7C3AED);
        c[ImGuiCol_SliderGrab]         = HEX(0x7C3AED);
        c[ImGuiCol_SliderGrabActive]   = HEX(0x6D28D9);

        c[ImGuiCol_Button]             = HEX(0x7C3AED);
        c[ImGuiCol_ButtonHovered]      = HEX(0x6D28D9);
        c[ImGuiCol_ButtonActive]       = HEX(0x5B21B6);

        c[ImGuiCol_Header]             = ColBgActive;
        c[ImGuiCol_HeaderHovered]      = ColBgHover;
        c[ImGuiCol_HeaderActive]       = ColBgActive;

        c[ImGuiCol_Separator]          = ColBorder;
        c[ImGuiCol_SeparatorHovered]   = ColBorderStrong;
        c[ImGuiCol_SeparatorActive]    = HEX(0x7C3AED);

        c[ImGuiCol_ResizeGrip]         = HEX(0xD9E2EC);
        c[ImGuiCol_ResizeGripHovered]  = HEX(0x7C3AED, 0.45f);
        c[ImGuiCol_ResizeGripActive]   = HEX(0x7C3AED, 0.80f);

        c[ImGuiCol_Tab]                = ColBgSubtle;
        c[ImGuiCol_TabHovered]         = ColBgHover;
        c[ImGuiCol_TabActive]          = ColBgCard;
        c[ImGuiCol_TabUnfocused]       = ColBgSubtle;
        c[ImGuiCol_TabUnfocusedActive] = ColBgCard;

        c[ImGuiCol_PlotLines]          = HEX(0x7C3AED);
        c[ImGuiCol_PlotLinesHovered]   = HEX(0xEC4899);
        c[ImGuiCol_PlotHistogram]      = HEX(0x7C3AED);
        c[ImGuiCol_PlotHistogramHovered] = HEX(0xEC4899);

        c[ImGuiCol_TableHeaderBg]      = ColBgSubtle;
        c[ImGuiCol_TableBorderStrong]  = ColBorder;
        c[ImGuiCol_TableBorderLight]   = HEX(0xEEF3F8);
        c[ImGuiCol_TableRowBg]         = ColBgCard;
        c[ImGuiCol_TableRowBgAlt]      = ColBgCardSoft;

        c[ImGuiCol_TextSelectedBg]     = HEX(0x7C3AED, 0.16f);
        c[ImGuiCol_DragDropTarget]     = HEX(0x7C3AED, 0.90f);
        c[ImGuiCol_NavHighlight]       = HEX(0x7C3AED);
        c[ImGuiCol_NavWindowingHighlight] = HEX(0x7C3AED, 0.7f);
        c[ImGuiCol_NavWindowingDimBg]  = HEX(0x0F172A, 0.2f);
        c[ImGuiCol_ModalWindowDimBg]   = HEX(0x0F172A, 0.35f);

        c[ImGuiCol_Text]               = ColTextPrimary;
        c[ImGuiCol_TextDisabled]       = ColTextFaint;
    }

    void applyDarkTheme() {
        applyTheme();
        ImGuiStyle& s = ImGui::GetStyle();
        s.FrameRounding = 10.0f;
    }

    void applyLightTheme() {
        applyTheme();
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
            case data::PRIORITY_LOW:      return "Low";
            case data::PRIORITY_MEDIUM:   return "Medium";
            case data::PRIORITY_HIGH:     return "High";
            case data::PRIORITY_CRITICAL: return "Critical";
        }
        return "?";
    }

    const char* statusLabel(data::Status s) {
        switch (s) {
            case data::STATUS_TODO:        return "To Do";
            case data::STATUS_IN_PROGRESS: return "In Progress";
            case data::STATUS_DONE:        return "Done";
            case data::STATUS_BLOCKED:     return "Blocked";
        }
        return "?";
    }

    BadgeStyle statusBadgeStyle(data::Status s) {
        switch (s) {
            case data::STATUS_TODO:
                return { "To Do",       HEX(0xF1F5F9), HEX(0x64748B), HEX(0x94A3B8) };
            case data::STATUS_IN_PROGRESS:
                return { "In Progress", HEX(0xFFFBEB), HEX(0xB45309), HEX(0xD97706) };
            case data::STATUS_DONE:
                return { "Done",        HEX(0xECFDF5), HEX(0x065F46), HEX(0x059669) };
            case data::STATUS_BLOCKED:
                return { "Blocked",     HEX(0xFEF2F2), HEX(0x991B1B), HEX(0xDC2626) };
        }
        return { "?", HEX(0xF1F5F9), HEX(0x64748B), HEX(0x94A3B8) };
    }

    BadgeStyle priorityBadgeStyle(data::Priority p) {
        switch (p) {
            case data::PRIORITY_LOW:
                return { "Low",      HEX(0xECFDF5), HEX(0x059669), HEX(0x059669) };
            case data::PRIORITY_MEDIUM:
                return { "Medium",   HEX(0xEFF6FF), HEX(0x2563EB), HEX(0x2563EB) };
            case data::PRIORITY_HIGH:
                return { "High",     HEX(0xFFF7ED), HEX(0xEA580C), HEX(0xEA580C) };
            case data::PRIORITY_CRITICAL:
                return { "Critical", HEX(0xFEF2F2), HEX(0xDC2626), HEX(0xDC2626) };
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
