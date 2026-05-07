#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    // Append entry to store.history with monotonic id and current timestamp.
    void pushHistory(data::TaskStore& store,
                     data::HistoryAction action,
                     int taskId,
                     const std::string& summary);

    // Recent entries first. Caller can cap with `limit` (0 = all).
    std::vector<const data::HistoryEntry*> recentHistory(
        const data::TaskStore& store, int limit);

    // Localised label for each action type.
    const char* historyActionLabel(data::HistoryAction action);

    // Wipe all entries (resets nextHistoryId to 1).
    void clearHistory(data::TaskStore& store);

}
