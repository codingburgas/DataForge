#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    bool loadStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage);

    bool saveStore(data::TaskStore& store,
                   const std::string& path,
                   std::string& errorMessage);

    bool saveStoreIfHasPath(data::TaskStore& store, std::string& errorMessage);

}
