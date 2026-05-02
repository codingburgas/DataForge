#include "ui/voice_input.h"
#include "ui/theme.h"
#include "platform/voice.h"
#include "imgui.h"

namespace ui {

    bool voiceMicButton(const char* id, char* buffer, int bufferSize,
                        float height) {
        bool active    = platform::voiceTargetIs(buffer);
        bool available = platform::isVoiceAvailable();

        ImVec4 bg, bgHover, bgActive, fg;
        const char* glyph;
        if (!available) {
            bg       = ColBgSubtle;
            bgHover  = ColBgSubtle;
            bgActive = ColBgSubtle;
            fg       = ColTextFaint;
            glyph    = "mic";
        } else if (active) {
            bg       = HEX(0xFEE2E2);
            bgHover  = HEX(0xFECACA);
            bgActive = HEX(0xFCA5A5);
            fg       = HEX(0xB91C1C);
            glyph    = "rec";
        } else {
            bg       = ColBgSubtle;
            bgHover  = ColBgHover;
            bgActive = ColBgActive;
            fg       = ColTextSecondary;
            glyph    = "mic";
        }

        ImGui::PushStyleColor(ImGuiCol_Button,        bg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  bgActive);
        ImGui::PushStyleColor(ImGuiCol_Text,          fg);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);

        float h = height > 0.0f ? height : ImGui::GetFrameHeight();
        ImVec2 size(48.0f, h);

        char composed[96];
        std::snprintf(composed, sizeof(composed), "%s###%s", glyph, id);

        bool clicked = ImGui::Button(composed, size);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if (ImGui::IsItemHovered()) {
            if (!available) {
                ImGui::SetTooltip("Voice input unavailable");
            } else if (active) {
                ImGui::SetTooltip("Stop dictation");
            } else {
                ImGui::SetTooltip("Dictate into this field");
            }
        }

        if (clicked && available) {
            if (active) {
                platform::clearVoiceTarget();
            } else {
                platform::setVoiceTarget(buffer, bufferSize);
            }
            return true;
        }
        return false;
    }

}
