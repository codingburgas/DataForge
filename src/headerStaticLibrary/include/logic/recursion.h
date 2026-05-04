#pragma once

#include "libraries.h"
#include "data/types.h"

// Tree-walking aggregations over the task hierarchy. Every function in
// this file is bounded by MAX_TREE_DEPTH so a corrupt parent chain in a
// loaded .dftasks file can never overflow the stack.
namespace logic {

    // Sum estimatedMinutes for the subtree rooted at `id`, including
    // the root itself. Returns 0 when the id is unknown.
    int   calculateTotalEstimatedMinutes(const data::TaskStore& store, int id);

    // Weighted completion ratio in [0,1]. Each node contributes its
    // done/not-done state weighted by its effective minutes (with a
    // floor of 1 so untimed leaves still count).
    float calculateWeightedCompletion(const data::TaskStore& store, int id);

    // Total number of descendants below `id` (does not include `id`).
    int   countDescendants(const data::TaskStore& store, int id);

    // Depth of the deepest path under `id`. Leaf returns 0.
    int   maxSubtreeDepth(const data::TaskStore& store, int id);

    // Pre-order list of every descendant id, appended to `out`.
    void  collectDescendantIds(const data::TaskStore& store,
                               int id,
                               std::vector<int>& out);

    // Iterative — counts tasks whose deadline has already passed and
    // whose status is not DONE. Tasks without a deadline are ignored.
    int   countOverdueTasks(const data::TaskStore& store, const data::Date& today);

}
