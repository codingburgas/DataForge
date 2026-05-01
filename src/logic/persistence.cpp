#include "logic/persistence.h"
#include "data/persistence.h"

namespace logic {

    bool loadStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage) {
        if (path.empty()) {
            errorMessage = "No file path provided.";
            return false;
        }
        return data::loadStoreFromFile(store, path, errorMessage);
    }

    bool saveStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage) {
        if (path.empty()) {
            errorMessage = "No file path provided.";
            return false;
        }
        return data::saveStoreToFile(store, path, errorMessage);
    }

    bool saveStoreIfHasPath(data::TaskStore& store,
                            std::string& errorMessage) {
        if (store.filePath.empty()) {
            errorMessage = "Store has no associated file path; use Save As.";
            return false;
        }
        return data::saveStoreToFile(store, store.filePath, errorMessage);
    }

}
