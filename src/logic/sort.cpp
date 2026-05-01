#include "logic/sort.h"
#include "logic/dates.h"

namespace logic {

    static bool sortsBefore(const data::Task& a,
                            const data::Task& b,
                            SortKey key) {
        if (key == SORT_KEY_PRIORITY) {
            return static_cast<int>(a.priority) >
                   static_cast<int>(b.priority);
        }
        return compareDates(a.deadline, b.deadline) < 0;
    }

    static void bubbleByKey(std::vector<data::Task>& v, SortKey key) {
        std::size_t n = v.size();
        if (n < 2) {
            return;
        }
        for (std::size_t pass = 0; pass + 1 < n; ++pass) {
            bool swapped = false;
            std::size_t lastIdx = n - 1 - pass;
            for (std::size_t i = 0; i < lastIdx; ++i) {
                if (sortsBefore(v[i + 1], v[i], key)) {
                    data::Task tmp = v[i];
                    v[i]     = v[i + 1];
                    v[i + 1] = tmp;
                    swapped = true;
                }
            }
            if (!swapped) {
                return;
            }
        }
    }

    static int partitionByKey(std::vector<data::Task>& v,
                              int lo, int hi,
                              SortKey key) {
        data::Task pivot = v[hi];
        int i = lo - 1;
        for (int j = lo; j < hi; ++j) {
            if (!sortsBefore(pivot, v[j], key)) {
                i += 1;
                data::Task tmp = v[i];
                v[i] = v[j];
                v[j] = tmp;
            }
        }
        data::Task tmp = v[i + 1];
        v[i + 1] = v[hi];
        v[hi]    = tmp;
        return i + 1;
    }

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
