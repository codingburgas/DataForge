#include "logic/history.h"

namespace logic {

    static std::string nowTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm local{};
    #if defined(_MSC_VER)
        localtime_s(&local, &now);
    #else
        std::tm* tmp = std::localtime(&now);
        if (tmp != nullptr) {
            local = *tmp;
        }
    #endif
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
                      local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                      local.tm_hour, local.tm_min, local.tm_sec);
        return buf;
    }

    void pushHistory(data::TaskStore& store,
                     data::HistoryAction action,
                     int taskId,
                     const std::string& summary) {
        if (store.nextHistoryId < 1) {
            store.nextHistoryId = 1;
        }
        data::HistoryEntry e;
        e.id        = store.nextHistoryId++;
        e.timestamp = nowTimestamp();
        e.action    = action;
        e.taskId    = taskId;
        e.summary   = summary;
        store.history.push_back(e);
        store.dirty = true;
    }

    std::vector<const data::HistoryEntry*> recentHistory(
        const data::TaskStore& store, int limit) {
        std::vector<const data::HistoryEntry*> out;
        out.reserve(store.history.size());
        for (std::size_t i = store.history.size(); i > 0; --i) {
            out.push_back(&store.history[i - 1]);
            if (limit > 0 && static_cast<int>(out.size()) >= limit) {
                break;
            }
        }
        return out;
    }

    const char* historyActionLabel(data::HistoryAction action) {
        switch (action) {
            case data::HIST_CREATE:        return "Create";
            case data::HIST_EDIT:          return "Edit";
            case data::HIST_DELETE:        return "Delete";
            case data::HIST_STATUS_CHANGE: return "Status";
            case data::HIST_LOAD:          return "Load";
            case data::HIST_SAVE:          return "Save";
            case data::HIST_DECISION:      return "Decision";
        }
        return "Action";
    }

    void clearHistory(data::TaskStore& store) {
        store.history.clear();
        store.nextHistoryId = 1;
        store.dirty = true;
    }

}
