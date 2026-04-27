#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    std::vector<int> searchTasksByTitleLinear(const data::TaskStore& store,
                                              const std::string& query);

    std::vector<int> filterAndSearch(const data::TaskStore& store,
                                     const std::string& query,
                                     int filterPriority,
                                     int filterStatus);

    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted,
                                int lo, int hi, int id);

    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted, int id);

    std::vector<data::Task> buildSortedByIdCopy(const data::TaskStore& store);

}
