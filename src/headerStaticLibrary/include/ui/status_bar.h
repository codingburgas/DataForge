#pragma once

#include "libraries.h"
#include "data/types.h"
#include "ui/ui_state.h"

namespace ui {

    void renderStatusBar(const data::TaskStore& store, const UiState& uiState);

}
