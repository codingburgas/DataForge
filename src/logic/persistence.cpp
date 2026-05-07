#include "logic/persistence.h"
#include "logic/history.h"
#include "data/persistence.h"

namespace logic {

    // Logic-tier wrapper around the data-tier file I/O. Adds the empty-
    // path guard so the UI can blindly forward whatever the file dialog
    // returned without having to validate it itself.
    bool loadStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage) {
        if (path.empty()) {
            errorMessage = "No file path provided.";
            return false;
        }
        if (!data::loadStoreFromFile(store, path, errorMessage)) {
            return false;
        }
        pushHistory(store, data::HIST_LOAD, -1, "Loaded " + path);
        store.dirty = false;
        return true;
    }

    bool saveStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage) {
        if (path.empty()) {
            errorMessage = "No file path provided.";
            return false;
        }
        pushHistory(store, data::HIST_SAVE, -1, "Saved " + path);
        return data::saveStoreToFile(store, path, errorMessage);
    }

    // Quick-save flow: only succeeds if the store already has a known
    // path. The UI uses this for Ctrl+S and falls back to Save As when
    // this returns false.
    bool saveStoreIfHasPath(data::TaskStore& store,
                            std::string& errorMessage) {
        if (store.filePath.empty()) {
            errorMessage = "Store has no associated file path; use Save As.";
            return false;
        }
        pushHistory(store, data::HIST_SAVE, -1, "Saved " + store.filePath);
        return data::saveStoreToFile(store, store.filePath, errorMessage);
    }

}
