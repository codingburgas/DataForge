#include "logic/benchmark.h"
#include "logic/sort.h"
#include "logic/dates.h"

namespace logic {

    // Generates a deterministic batch of tasks for the benchmark panel.
    // Same seed produces the same dataset on every run so the user can
    // re-run the comparison and trust that any timing change is the
    // algorithm, not the input.
    std::vector<data::Task> generateSyntheticTasks(int count, unsigned seed) {
        std::vector<data::Task> v;
        if (count <= 0) {
            return v;
        }
        v.reserve(static_cast<std::size_t>(count));
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> priDist(
            data::PRIORITY_LOW, data::PRIORITY_CRITICAL);
        std::uniform_int_distribution<int> yearOffsetDist(-1, 3);
        std::uniform_int_distribution<int> monthDist(1, 12);
        // Cap day at 28 to avoid month-end edge cases for the random data.
        std::uniform_int_distribution<int> dayDist(1, 28);
        std::uniform_int_distribution<int> minDist(5, 600);
        // Roughly 80% of synthetic tasks get a deadline; the rest stay
        // zero-dated to exercise the "no deadline" code paths.
        std::uniform_int_distribution<int> hasDeadlineDist(0, 9);

        data::Date today = logic::today();
        for (int i = 0; i < count; ++i) {
            data::Task t{};
            t.id               = i + 1;
            t.parentId         = -1;
            t.title            = "Synthetic task " + std::to_string(i + 1);
            t.description      = std::string();
            t.priority         = static_cast<data::Priority>(priDist(rng));
            t.status           = data::STATUS_TODO;
            if (hasDeadlineDist(rng) < 8) {
                data::Date d{};
                d.year  = today.year + yearOffsetDist(rng);
                d.month = monthDist(rng);
                d.day   = dayDist(rng);
                t.deadline = d;
            } else {
                t.deadline = logic::zeroDate();
            }
            t.estimatedMinutes = minDist(rng);
            t.actualMinutes    = 0;
            t.createdAt        = today;
            t.updatedAt        = today;
            v.push_back(t);
        }
        return v;
    }

    // Times bubble sort and quicksort on identical input. We copy the
    // generated batch into two vectors so each algorithm starts from the
    // exact same unsorted state — otherwise the second one would benefit
    // from already being partially sorted by the first run.
    SortBenchmarkResult benchmarkSortAlgorithms(int itemCount, unsigned seed) {
        SortBenchmarkResult r{};
        r.itemCount = itemCount;
        r.bubbleMs  = 0.0;
        r.quickMs   = 0.0;
        if (itemCount <= 0) {
            return r;
        }
        std::vector<data::Task> bubbleCopy =
            generateSyntheticTasks(itemCount, seed);
        std::vector<data::Task> quickCopy = bubbleCopy;

        auto t0 = std::chrono::high_resolution_clock::now();
        sortTasksByPriorityBubble(bubbleCopy);
        auto t1 = std::chrono::high_resolution_clock::now();
        sortTasksByDeadlineQuick(quickCopy);
        auto t2 = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> bubbleDur = t1 - t0;
        std::chrono::duration<double, std::milli> quickDur  = t2 - t1;
        r.bubbleMs = bubbleDur.count();
        r.quickMs  = quickDur.count();
        return r;
    }

}
