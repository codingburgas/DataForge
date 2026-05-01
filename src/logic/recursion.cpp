#include "logic/recursion.h"
#include "logic/tasks.h"
#include "data/store.h"
#include "data/date.h"

namespace logic {

    static int calcTotalMinutesInner(const data::TaskStore& store,
                                     int id,
                                     int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0;
        }
        const data::Task* t = data::findTaskInStoreConst(store, id);
        if (t == nullptr) {
            return 0;
        }
        int total = t->estimatedMinutes;
        std::vector<int> children = childTaskIds(store, id);
        for (std::size_t i = 0; i < children.size(); ++i) {
            total += calcTotalMinutesInner(store, children[i], depth + 1);
        }
        return total;
    }

    int calculateTotalEstimatedMinutes(const data::TaskStore& store, int id) {
        return calcTotalMinutesInner(store, id, 0);
    }

    static int effectiveMinutes(const data::Task& t) {
        return t.estimatedMinutes > 0 ? t.estimatedMinutes : 1;
    }

    static float calcWeightedInner(const data::TaskStore& store,
                                   int id,
                                   int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0.0f;
        }
        const data::Task* t = data::findTaskInStoreConst(store, id);
        if (t == nullptr) {
            return 0.0f;
        }
        std::vector<int> children = childTaskIds(store, id);
        int   ownWeight     = effectiveMinutes(*t);
        float ownCompletion = (t->status == data::STATUS_DONE) ? 1.0f : 0.0f;
        if (children.empty()) {
            return ownCompletion;
        }
        long long totalWeight = ownWeight;
        double    weightedSum = static_cast<double>(ownCompletion) * ownWeight;
        for (std::size_t i = 0; i < children.size(); ++i) {
            const data::Task* c =
                data::findTaskInStoreConst(store, children[i]);
            if (c == nullptr) {
                continue;
            }
            int   w = effectiveMinutes(*c);
            float r = calcWeightedInner(store, children[i], depth + 1);
            totalWeight += w;
            weightedSum += static_cast<double>(r) * w;
        }
        if (totalWeight <= 0) {
            return 0.0f;
        }
        return static_cast<float>(weightedSum / static_cast<double>(totalWeight));
    }

    float calculateWeightedCompletion(const data::TaskStore& store, int id) {
        return calcWeightedInner(store, id, 0);
    }

    static int countDescInner(const data::TaskStore& store,
                              int id,
                              int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0;
        }
        std::vector<int> children = childTaskIds(store, id);
        int count = 0;
        for (std::size_t i = 0; i < children.size(); ++i) {
            count += 1 + countDescInner(store, children[i], depth + 1);
        }
        return count;
    }

    int countDescendants(const data::TaskStore& store, int id) {
        return countDescInner(store, id, 0);
    }

    static int depthInner(const data::TaskStore& store, int id, int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0;
        }
        std::vector<int> children = childTaskIds(store, id);
        if (children.empty()) {
            return 0;
        }
        int best = 0;
        for (std::size_t i = 0; i < children.size(); ++i) {
            int d = 1 + depthInner(store, children[i], depth + 1);
            if (d > best) {
                best = d;
            }
        }
        return best;
    }

    int maxSubtreeDepth(const data::TaskStore& store, int id) {
        return depthInner(store, id, 0);
    }

    static void collectInner(const data::TaskStore& store,
                             int id,
                             int depth,
                             std::vector<int>& out) {
        if (depth > MAX_TREE_DEPTH) {
            return;
        }
        std::vector<int> children = childTaskIds(store, id);
        for (std::size_t i = 0; i < children.size(); ++i) {
            out.push_back(children[i]);
            collectInner(store, children[i], depth + 1, out);
        }
    }

    void collectDescendantIds(const data::TaskStore& store,
                              int id,
                              std::vector<int>& out) {
        collectInner(store, id, 0, out);
    }

    int countOverdueTasks(const data::TaskStore& store,
                          const data::Date& today) {
        int count = 0;
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            const data::Task& t = store.tasks[i];
            if (data::isDateZero(t.deadline)) {
                continue;
            }
            if (t.status == data::STATUS_DONE) {
                continue;
            }
            if (data::compareDates(t.deadline, today) < 0) {
                count += 1;
            }
        }
        return count;
    }

}
