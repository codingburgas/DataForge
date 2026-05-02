#include "logic/search.h"

namespace logic {

    static char toLowerAscii(char c) {
        if (c >= 'A' && c <= 'Z') {
            return static_cast<char>(c - 'A' + 'a');
        }
        return c;
    }

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

    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted,
                                int lo, int hi, int id) {
        if (lo > hi) {
            return -1;
        }
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

    int findTaskIndexByIdBinary(const std::vector<data::Task>& sorted, int id) {
        if (sorted.empty()) {
            return -1;
        }
        return findTaskIndexByIdBinary(sorted, 0,
                                       static_cast<int>(sorted.size()) - 1,
                                       id);
    }

    std::vector<data::Task> buildSortedByIdCopy(const data::TaskStore& store) {
        std::vector<data::Task> copy = store.tasks;
        std::sort(copy.begin(), copy.end(),
                  [](const data::Task& a, const data::Task& b) {
                      return a.id < b.id;
                  });
        return copy;
    }

}
