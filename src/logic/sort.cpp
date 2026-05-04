#include "logic/sort.h"
#include "logic/dates.h"

namespace logic {

    // Single ordering predicate shared by both algorithms so behaviour
    // stays identical regardless of which sort the user picks. Priority
    // sorts DESC (Critical first), deadlines sort ASC (soonest first).
    static bool sortsBefore(const data::Task& a,
                            const data::Task& b,
                            SortKey key) {
        if (key == SORT_KEY_PRIORITY) {
            return static_cast<int>(a.priority) >
                   static_cast<int>(b.priority);
        }
        return compareDates(a.deadline, b.deadline) < 0;
    }

    // Classic bubble sort with the optimisation that an untouched pass
    // means the array is already sorted, so we can exit early. O(n^2)
    // worst case, O(n) on already-sorted input — kept hand-rolled for
    // teaching purposes and to compare against quicksort in the panel.
    static void bubbleByKey(std::vector<data::Task>& v, SortKey key) {
        std::size_t n = v.size();
        if (n < 2) {
            return;
        }
        for (std::size_t pass = 0; pass + 1 < n; ++pass) {
            bool swapped = false;
            // Last `pass` elements are guaranteed to be in their final
            // position, so we can shrink the inner loop each pass.
            std::size_t lastIdx = n - 1 - pass;
            for (std::size_t i = 0; i < lastIdx; ++i) {
                if (sortsBefore(v[i + 1], v[i], key)) {
                    std::swap(v[i], v[i + 1]);
                    swapped = true;
                }
            }
            if (!swapped) {
                return;
            }
        }
    }

    // Lomuto partition scheme: pivot is the rightmost element, `i` tracks
    // the boundary of the "less than pivot" region. Returns the final
    // resting index of the pivot after placement.
    static int partitionByKey(std::vector<data::Task>& v,
                              int lo, int hi,
                              SortKey key) {
        data::Task pivot = v[hi];
        int i = lo - 1;
        for (int j = lo; j < hi; ++j) {
            // !sortsBefore(pivot, v[j]) means v[j] is <= pivot in our
            // ordering, so it belongs on the left side.
            if (!sortsBefore(pivot, v[j], key)) {
                i += 1;
                std::swap(v[i], v[j]);
            }
        }
        std::swap(v[i + 1], v[hi]);
        return i + 1;
    }

    // Recursive quicksort. Average O(n log n), worst O(n^2) on already-
    // sorted input thanks to the naive last-element pivot — that is
    // intentional and visible in the benchmark panel as a teaching point.
    static void quickByKey(std::vector<data::Task>& v,
                           int lo, int hi,
                           SortKey key) {
        if (lo >= hi || lo < 0) {
            return;
        }
        int p = partitionByKey(v, lo, hi, key);
        quickByKey(v, lo, p - 1, key);
        quickByKey(v, p + 1, hi, key);
    }

    void sortTasksByPriorityBubble(std::vector<data::Task>& v) {
        bubbleByKey(v, SORT_KEY_PRIORITY);
    }

    void sortTasksByDeadlineQuick(std::vector<data::Task>& v, int lo, int hi) {
        quickByKey(v, lo, hi, SORT_KEY_DEADLINE);
    }

    void sortTasksByDeadlineQuick(std::vector<data::Task>& v) {
        if (v.size() < 2) {
            return;
        }
        quickByKey(v, 0, static_cast<int>(v.size()) - 1, SORT_KEY_DEADLINE);
    }

    // Public dispatcher used by the UI when the user picks an algorithm
    // and a key independently from the toolbar.
    void sortTasks(std::vector<data::Task>& v,
                   SortKey key,
                   SortAlgorithm algo) {
        if (algo == SORT_ALGO_BUBBLE) {
            bubbleByKey(v, key);
            return;
        }
        if (v.size() < 2) {
            return;
        }
        quickByKey(v, 0, static_cast<int>(v.size()) - 1, key);
    }

}
