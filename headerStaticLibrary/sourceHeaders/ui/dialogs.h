#pragma once

#include "libraries.h"
#include "data/types.h"
#include "ui/ui_state.h"

namespace ui {

    void openCreateDialog(UiState& uiState, int parentId);
    void openEditDialog(UiState& uiState, const data::Task& t);
    void openConfirmDelete(UiState& uiState, int taskId);

    void renderAddEditDialog(data::TaskStore& store, UiState& uiState);
    void renderConfirmDeleteDialog(data::TaskStore& store, UiState& uiState);
    void renderDirtyExitConfirmDialog(data::TaskStore& store, UiState& uiState);
    void renderAboutPopup(UiState& uiState);

}
