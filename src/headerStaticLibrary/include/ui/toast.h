#pragma once

#include "libraries.h"
#include "ui/ui_state.h"

namespace ui {

    constexpr double TOAST_LIFETIME_SECONDS = 4.0;

    void pushToast(UiState& uiState, const std::string& message);
    void renderToast(UiState& uiState);

}
