#pragma once

#include "libraries.h"
#include "data/types.h"

// Two complementary search paths:
//   * linear scan over titles for the human-facing search bar
//   * recursive binary search over a sorted-by-id snapshot for fast
//     internal lookups (e.g. resolving a UI selection to a task)
namespace logic {

    // O(n*m). Case-insensitive substring match on the title field.
    // Returns the matching task ids in store order. Empty query matches
    // every task, which is the expected search-bar behaviour.
    std::vector<int> searchTasksByTitleLinear(const data::TaskStore& store,
                                              const std::string& query);

    // Filter by priority/status first (-1 means "no filter"), then run
    // the substring match within the narrowed set. Single pass, O(n*m).
    std::vector<int> filterAndSearch(const data::TaskStore& store,
                                     const std::string& query,
                                     int filterPriority,
                                     int filterStatus);

    // Recursive binary search. Pre-condition: `sorted` is in ascending
    // id order (use buildSortedByIdCopy below). O(log n).
    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted,
                                int lo, int hi, int id);

    // Convenience overload — drives the recursion across the full range.
    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted, int id);

    // Returns a copy of the store's tasks ordered by ascending id, ready
    // for binary search. We do not keep the store itself sorted by id
    // because the table view wants insertion order.
    std::vector<data::Task> buildSortedByIdCopy(const data::TaskStore& store);

}
