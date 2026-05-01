#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    enum SortKey {
        SORT_KEY_PRIORITY = 0,
        SORT_KEY_DEADLINE = 1
    };

    enum SortAlgorithm {
        SORT_ALGO_BUBBLE = 0,
        SORT_ALGO_QUICK  = 1
    };

    void sortTasksByPriorityBubble(std::vector<data::Task>& v);
    void sortTasksByDeadlineQuick(std::vector<data::Task>& v, int lo, int hi);
    void sortTasksByDeadlineQuick(std::vector<data::Task>& v);

    void sortTasks(std::vector<data::Task>& v,
                   SortKey key,
                   SortAlgorithm algo);

}
