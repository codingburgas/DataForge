#pragma once

#include "libraries.h"
#include "data/types.h"

// Sorting algorithms the user can pick between in the UI. Both are
// implemented by hand (no std::sort) so the algorithm choice is visible.
namespace logic {

    // Which task field drives the comparison.
    enum SortKey {
        SORT_KEY_PRIORITY = 0,
        SORT_KEY_DEADLINE = 1
    };

    // Which algorithm to run. Bubble is iterative and stable on tiny
    // inputs; Quick is recursive and much faster on large inputs.
    enum SortAlgorithm {
        SORT_ALGO_BUBBLE = 0,
        SORT_ALGO_QUICK  = 1
    };

    // Iterative bubble sort by priority (Critical first).
    void sortTasksByPriorityBubble(std::vector<data::Task>& v);

    // Recursive quicksort over the inclusive range [lo, hi].
    void sortTasksByDeadlineQuick(std::vector<data::Task>& v, int lo, int hi);
    // Convenience overload that sorts the entire vector by deadline.
    void sortTasksByDeadlineQuick(std::vector<data::Task>& v);

    // Public dispatcher: pick a key and algorithm independently.
    void sortTasks(std::vector<data::Task>& v,
                   SortKey key,
                   SortAlgorithm algo);

}
