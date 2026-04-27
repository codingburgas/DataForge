#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    int   calculateTotalEstimatedMinutes(const data::TaskStore& store, int id);
    float calculateWeightedCompletion(const data::TaskStore& store, int id);
    int   countDescendants(const data::TaskStore& store, int id);
    int   maxSubtreeDepth(const data::TaskStore& store, int id);
    void  collectDescendantIds(const data::TaskStore& store,
                               int id,
                               std::vector<int>& out);

    int   countOverdueTasks(const data::TaskStore& store, const data::Date& today);

}
