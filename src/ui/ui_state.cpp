#include "ui/ui_state.h"
#include "logic/dates.h"

namespace ui {

    double nowSeconds() {
        using namespace std::chrono;
        auto epoch = steady_clock::now().time_since_epoch();
        return duration_cast<duration<double>>(epoch).count();
    }

    void clearEditBuffers(EditBuffers& eb) {
        eb.titleBuf   [0] = '\0';
        eb.descBuf    [0] = '\0';
        eb.deadlineBuf[0] = '\0';
        eb.estimateBuf[0] = '\0';
        eb.actualBuf  [0] = '\0';
        eb.priority       = data::PRIORITY_MEDIUM;
        eb.status         = data::STATUS_TODO;
        eb.parentId       = -1;
    }

    static void copyTo(char* dst, std::size_t cap, const std::string& s) {
        std::size_t n = s.size() < cap - 1 ? s.size() : cap - 1;
        std::memcpy(dst, s.data(), n);
        dst[n] = '\0';
    }

    static void copyIntTo(char* dst, std::size_t cap, int value) {
        std::snprintf(dst, cap, "%d", value);
    }

    void loadEditBuffersFromTask(EditBuffers& eb, const data::Task& t) {
        copyTo   (eb.titleBuf,    TITLE_BUF_SIZE,   t.title);
        copyTo   (eb.descBuf,     DESC_BUF_SIZE,    t.description);
        copyTo   (eb.deadlineBuf, DATE_BUF_SIZE,    logic::formatDate(t.deadline));
        copyIntTo(eb.estimateBuf, NUMERIC_BUF_SIZE, t.estimatedMinutes);
        copyIntTo(eb.actualBuf,   NUMERIC_BUF_SIZE, t.actualMinutes);
        eb.priority = static_cast<int>(t.priority);
        eb.status   = static_cast<int>(t.status);
        eb.parentId = t.parentId;
    }

    void applyEditBuffersToTask(const EditBuffers& eb, data::Task& t) {
        t.title       = std::string(eb.titleBuf);
        t.description = std::string(eb.descBuf);
        data::Date d  = logic::zeroDate();
        logic::parseDate(std::string(eb.deadlineBuf), d);
        t.deadline         = d;
        t.estimatedMinutes = std::atoi(eb.estimateBuf);
        t.actualMinutes    = std::atoi(eb.actualBuf);
        int p = eb.priority;
        if (p < data::PRIORITY_LOW)       p = data::PRIORITY_LOW;
        if (p > data::PRIORITY_CRITICAL)  p = data::PRIORITY_CRITICAL;
        int s = eb.status;
        if (s < data::STATUS_TODO)    s = data::STATUS_TODO;
        if (s > data::STATUS_BLOCKED) s = data::STATUS_BLOCKED;
        t.priority = static_cast<data::Priority>(p);
        t.status   = static_cast<data::Status>(s);
        t.parentId = eb.parentId;
    }

    void initUiState(UiState& ui) {
        ui.selectedTaskId   = -1;
        ui.editingTaskId    = -1;
        ui.pendingDeleteId  = -1;
        ui.parentForNewTask = -1;

        ui.searchBuffer[0] = '\0';

        ui.filterPriority = -1;
        ui.filterStatus   = -1;

        ui.sortKey  = logic::SORT_KEY_PRIORITY;
        ui.sortAlgo = logic::SORT_ALGO_BUBBLE;

        ui.showStatsPanel       = false;
        ui.showBenchmarkPanel   = false;
        ui.showAboutPopup       = false;
        ui.showAddEditDialog    = false;
        ui.showConfirmDelete    = false;
        ui.showDirtyExitConfirm = false;

        ui.useDarkTheme    = true;
        ui.requestedQuit   = false;
        ui.triggeredSave   = false;
        ui.triggeredSaveAs = false;
        ui.triggeredOpen   = false;
        ui.triggeredNew    = false;

        ui.pendingFilePath.clear();

        ui.toastMessage.clear();
        ui.toastUntilSeconds = 0.0;

        ui.undoSnapshot     = data::TaskStore{};
        ui.hasUndoSnapshot  = false;

        ui.benchmarkItemCount = 1000;
        ui.lastBenchmark.reset();

        ui.lastValidationOk = true;
        ui.lastValidationMessage.clear();

        clearEditBuffers(ui.edit);
    }

}
