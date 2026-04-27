#include "ui/toast.h"
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
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImVec2 pos;
        pos.x = vp->WorkPos.x + vp->WorkSize.x - 20.0f;
        pos.y = vp->WorkPos.y + vp->WorkSize.y - 60.0f;
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.85f);
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("##Toast", nullptr, flags)) {
            ImGui::TextUnformatted(uiState.toastMessage.c_str());
        }
        ImGui::End();
    }

}
