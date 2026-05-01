#include "ui/toast.h"
#include "ui/theme.h"
#include "imgui.h"

namespace ui {

    void pushToast(UiState& uiState, const std::string& message) {
        uiState.toastMessage      = message;
        uiState.toastUntilSeconds = nowSeconds() + TOAST_LIFETIME_SECONDS;
    }

    void renderToast(UiState& uiState) {
        if (uiState.toastMessage.empty()) {
            return;
        }

        double t = nowSeconds();
        if (t >= uiState.toastUntilSeconds) {
            uiState.toastMessage.clear();
            return;
        }

        float remaining = static_cast<float>(uiState.toastUntilSeconds - t);
        float alpha = remaining < 0.5f ? remaining / 0.5f : 1.0f;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImVec2 size = ImVec2(320.0f, 60.0f);
        ImVec2 pos = ImVec2(
            vp->WorkPos.x + vp->WorkSize.x - size.x - 18.0f,
            vp->WorkPos.y + vp->WorkSize.y - size.y - 46.0f);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowBgAlpha(alpha);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ColBgCard);
        ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        if (ImGui::Begin("##Toast", nullptr, flags)) {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 min = ImGui::GetWindowPos();
            ImVec2 max = ImVec2(min.x + size.x, min.y + size.y);
            drawSoftShadow(dl, min, max, 18.0f,
                           IM_COL32(15, 23, 42, static_cast<int>(18 * alpha)),
                           ImVec2(0.0f, 8.0f));
            drawGradientRect(dl, ImVec2(min.x + 10.0f, min.y + 14.0f),
                             ImVec2(min.x + 50.0f, min.y + 46.0f), 12.0f);
            dl->AddLine(ImVec2(min.x + 23.0f, min.y + 31.0f),
                        ImVec2(min.x + 29.0f, min.y + 37.0f),
                        IM_COL32(255, 255, 255, 255), 2.0f);
            dl->AddLine(ImVec2(min.x + 29.0f, min.y + 37.0f),
                        ImVec2(min.x + 38.0f, min.y + 24.0f),
                        IM_COL32(255, 255, 255, 255), 2.0f);

            ImGui::SetCursorPos(ImVec2(60.0f, 14.0f));
            ImGui::PushFont(fontUiSemibold());
            ImGui::TextColored(ColTextPrimary, "Update");
            ImGui::PopFont();
            ImGui::SetCursorPos(ImVec2(60.0f, 34.0f));
            ImGui::TextColored(ImVec4(ColTextMuted.x, ColTextMuted.y, ColTextMuted.z, alpha),
                               "%s", uiState.toastMessage.c_str());
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

}
