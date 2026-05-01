#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    struct SortBenchmarkResult {
        int    itemCount;
        double bubbleMs;
        double quickMs;
    };

    SortBenchmarkResult benchmarkSortAlgorithms(int itemCount, unsigned seed);

    std::vector<data::Task> generateSyntheticTasks(int count, unsigned seed);

}
