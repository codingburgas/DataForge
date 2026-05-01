#pragma once

#include "libraries.h"
#include "data/types.h"

namespace data {

    bool loadStoreFromFile(TaskStore& store,
                           const std::string& path,
                           std::string& errorMessage);

    bool saveStoreToFile(TaskStore& store,
                         const std::string& path,
                         std::string& errorMessage);

    bool storeToText(const TaskStore& store, std::string& out);
    bool storeFromText(const std::string& text,
                       TaskStore& store,
                       std::string& errorMessage);

}
