#include "logic/recursion.h"
#include "logic/tasks.h"
#include "data/store.h"
#include "data/date.h"

namespace logic {

    // All recursive walks below carry an explicit `depth` counter and bail
    // when MAX_TREE_DEPTH is exceeded. This is a defence against malformed
    // .dftasks files that could encode a parent cycle the validator missed,
    // turning an unbounded recursion into a bounded no-op rather than a
    // stack overflow.

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
        // Sum own estimate plus the sum of every descendant's estimate.
        int total = t->estimatedMinutes;
        std::vector<int> children = childTaskIds(store, id);
        for (int childId : children) {
            total += calcTotalMinutesInner(store, childId, depth + 1);
        }
        return total;
    }

    int calculateTotalEstimatedMinutes(const data::TaskStore& store, int id) {
        return calcTotalMinutesInner(store, id, 0);
    }

    // We weight each node by its estimated minutes so a 10-minute leaf
    // does not count as much as a 5-hour leaf. A zero estimate is bumped
    // to 1 so leaves without an estimate still influence the rollup
    // instead of disappearing from the average.
    static int effectiveMinutes(const data::Task& t) {
        return t.estimatedMinutes > 0 ? t.estimatedMinutes : 1;
    }

    // Returns weighted completion in [0,1]. A node contributes its own
    // done/not-done state as a value, weighted by its effective minutes,
    // and each child contributes its recursive result weighted by the
    // child's effective minutes. Childless nodes return their own state
    // directly so the recursion has a clean base case.
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
        // Use long long / double for the running totals so deep trees with
        // large minute estimates do not overflow or accumulate float drift.
        long long totalWeight = ownWeight;
        double    weightedSum = static_cast<double>(ownCompletion) * ownWeight;
        for (int childId : children) {
            const data::Task* c =
                data::findTaskInStoreConst(store, childId);
            if (c == nullptr) {
                continue;
            }
            int   w = effectiveMinutes(*c);
            float r = calcWeightedInner(store, childId, depth + 1);
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

    // Counts every descendant but NOT the root itself, matching how the
    // delete-confirmation dialog phrases "this task plus N descendants".
    static int countDescInner(const data::TaskStore& store,
                              int id,
                              int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0;
        }
        std::vector<int> children = childTaskIds(store, id);
        int count = 0;
        for (int childId : children) {
            count += 1 + countDescInner(store, childId, depth + 1);
        }
        return count;
    }

    int countDescendants(const data::TaskStore& store, int id) {
        return countDescInner(store, id, 0);
    }

    // Depth of the tallest path below `id`. A leaf has depth 0; a node
    // with a single leaf child has depth 1. Used by the stats panel and
    // by the "would adding here exceed max depth?" validator.
    static int depthInner(const data::TaskStore& store, int id, int depth) {
        if (depth > MAX_TREE_DEPTH) {
            return 0;
        }
        std::vector<int> children = childTaskIds(store, id);
        if (children.empty()) {
            return 0;
        }
        int best = 0;
        for (int childId : children) {
            int d = 1 + depthInner(store, childId, depth + 1);
            if (d > best) {
                best = d;
            }
        }
        return best;
    }

    int maxSubtreeDepth(const data::TaskStore& store, int id) {
        return depthInner(store, id, 0);
    }

    // Pre-order collection (parent before children would be more natural
    // here, but the cascade-delete caller adds the root explicitly first,
    // so we only emit descendants).
    static void collectInner(const data::TaskStore& store,
                             int id,
                             int depth,
                             std::vector<int>& out) {
        if (depth > MAX_TREE_DEPTH) {
            return;
        }
        std::vector<int> children = childTaskIds(store, id);
        for (int childId : children) {
            out.push_back(childId);
            collectInner(store, childId, depth + 1, out);
        }
    }

    void collectDescendantIds(const data::TaskStore& store,
                              int id,
                              std::vector<int>& out) {
        collectInner(store, id, 0, out);
    }

    // Iterative — counts overdue across the whole store, not a subtree.
    // Tasks with no deadline (zero date) are skipped, as are completed
    // tasks: a done task can never be "overdue" in a useful sense.
    int countOverdueTasks(const data::TaskStore& store,
                          const data::Date& today) {
        int count = 0;
        for (const data::Task& t : store.tasks) {
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
