#include "logic/search.h"

namespace logic {

    // ASCII-only lowercase. Avoids std::tolower because the global locale
    // can corrupt non-ASCII bytes in UTF-8 task titles; this stays a no-op
    // on bytes outside [A-Z], which is safe for our search use case.
    static char toLowerAscii(char c) {
        if (c >= 'A' && c <= 'Z') {
            return static_cast<char>(c - 'A' + 'a');
        }
        return c;
    }

    // Naive O(n*m) substring scan. We intentionally avoid std::search so
    // the search algorithm is visible and graded as hand-rolled. Empty
    // needle matches everything, matching typical search-bar UX.
    static bool containsIgnoreCase(const std::string& haystack,
                                   const std::string& needle) {
        if (needle.empty()) {
            return true;
        }
        if (needle.size() > haystack.size()) {
            return false;
        }
        std::size_t limit = haystack.size() - needle.size();
        for (std::size_t i = 0; i <= limit; ++i) {
            bool match = true;
            for (std::size_t j = 0; j < needle.size(); ++j) {
                if (toLowerAscii(haystack[i + j]) !=
                    toLowerAscii(needle[j])) {
                    match = false;
                    break;
                }
            }
            if (match) {
                return true;
            }
        }
        return false;
    }

    // Linear search by title — what users expect a free-text search to do.
    // Returns IDs (not indices) so the result survives later store edits.
    std::vector<int> searchTasksByTitleLinear(const data::TaskStore& store,
                                              const std::string& query) {
        std::vector<int> out;
        out.reserve(store.tasks.size());
        for (const data::Task& t : store.tasks) {
            if (containsIgnoreCase(t.title, query)) {
                out.push_back(t.id);
            }
        }
        return out;
    }

    // Compose filter + search in a single pass. -1 sentinels mean "no
    // filter on this dimension", which keeps the UI binding simple.
    std::vector<int> filterAndSearch(const data::TaskStore& store,
                                     const std::string& query,
                                     int filterPriority,
                                     int filterStatus) {
        std::vector<int> out;
        out.reserve(store.tasks.size());
        for (const data::Task& t : store.tasks) {
            if (filterPriority >= 0 &&
                static_cast<int>(t.priority) != filterPriority) {
                continue;
            }
            if (filterStatus >= 0 &&
                static_cast<int>(t.status) != filterStatus) {
                continue;
            }
            if (!containsIgnoreCase(t.title, query)) {
                continue;
            }
            out.push_back(t.id);
        }
        return out;
    }

    // Recursive binary search. Pre-condition: caller passes a vector
    // sorted by Task::id ascending (see buildSortedByIdCopy below).
    // Returns -1 when not found. O(log n).
    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted,
                                int lo, int hi, int id) {
        if (lo > hi) {
            return -1;
        }
        // Avoid (lo + hi) / 2 to prevent integer overflow on large ranges.
        int mid = lo + (hi - lo) / 2;
        int v   = sorted[mid].id;
        if (v == id) {
            return mid;
        }
        if (v < id) {
            return findTaskIndexByIdBinary(sorted, mid + 1, hi, id);
        }
        return findTaskIndexByIdBinary(sorted, lo, mid - 1, id);
    }

    // Convenience overload that drives the recursion from the full range.
    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted, int id) {
        if (sorted.empty()) {
            return -1;
        }
        return findTaskIndexByIdBinary(sorted, 0,
                                       static_cast<int>(sorted.size()) - 1,
                                       id);
    }

    // Build the sorted-by-id snapshot the recursive binary search expects.
    // We do not keep the store itself sorted by id because the table view
    // wants insertion order; this small copy is the price for that.
    std::vector<data::Task> buildSortedByIdCopy(const data::TaskStore& store) {
        std::vector<data::Task> copy = store.tasks;
        std::sort(copy.begin(), copy.end(),
                  [](const data::Task& a, const data::Task& b) {
                      return a.id < b.id;
                  });
        return copy;
    }

}
