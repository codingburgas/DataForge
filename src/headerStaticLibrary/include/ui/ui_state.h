#pragma once

#include "libraries.h"
#include "data/types.h"
#include "logic/sort.h"
#include "logic/benchmark.h"
#include "ui/i18n.h"

namespace ui {

    enum NavItem {
        NAV_OVERVIEW = 0,
        NAV_TASKS = 1,
        NAV_ANALYTICS = 2,
        NAV_BENCHMARK = 3,
        NAV_HELP = 4
    };

    constexpr int SEARCH_BUF_SIZE   = 256;
    constexpr int TITLE_BUF_SIZE    = 256;
    constexpr int DESC_BUF_SIZE     = 2048;
    constexpr int DATE_BUF_SIZE     = 16;
    constexpr int NUMERIC_BUF_SIZE  = 16;

    struct EditBuffers {
        char titleBuf   [TITLE_BUF_SIZE];
        char descBuf    [DESC_BUF_SIZE];
        char deadlineBuf[DATE_BUF_SIZE];
        char estimateBuf[NUMERIC_BUF_SIZE];
        char actualBuf  [NUMERIC_BUF_SIZE];
        int  priority;
        int  status;
        int  parentId;
    };

    struct UiState {
        int  selectedTaskId;
        int  editingTaskId;
        int  pendingDeleteId;
        int  parentForNewTask;

        char searchBuffer[SEARCH_BUF_SIZE];

        int  filterPriority;
        int  filterStatus;

        logic::SortKey       sortKey;
        logic::SortAlgorithm sortAlgo;

        bool showStatsPanel;
        bool showBenchmarkPanel;
        bool showAboutPopup;
        bool showAddEditDialog;
        bool showConfirmDelete;
        bool showDirtyExitConfirm;

        bool useDarkTheme;
        bool requestedQuit;
        bool triggeredSave;
        bool triggeredSaveAs;
        bool triggeredOpen;
        bool triggeredOpenPath;
        bool triggeredNew;

        std::string pendingFilePath;

        std::string toastMessage;
        double      toastUntilSeconds;

        data::TaskStore undoSnapshot;
        bool            hasUndoSnapshot;

        int benchmarkItemCount;
        std::optional<logic::SortBenchmarkResult> lastBenchmark;

        bool        lastValidationOk;
        std::string lastValidationMessage;

        bool    sidebarExpanded;
        NavItem activeNavItem;

        Language language;

        EditBuffers edit;
    };

    void initUiState(UiState& ui);
    void clearEditBuffers(EditBuffers& eb);
    void loadEditBuffersFromTask(EditBuffers& eb, const data::Task& t);
    void applyEditBuffersToTask(const EditBuffers& eb, data::Task& t);

    double nowSeconds();

}
