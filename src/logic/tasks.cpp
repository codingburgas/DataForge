#include "logic/tasks.h"
#include "logic/recursion.h"
#include "logic/dates.h"
#include "data/store.h"

namespace logic {

    // Thin pass-through so callers in the UI never need to include the
    // data layer directly — keeps the three-tier discipline intact.
    data::TaskStore createEmptyStore() {
        return data::createEmptyStore();
    }

    // A "root" is any task whose parentId is either -1 (explicitly none)
    // or points at an id that no longer exists in the store. The second
    // case happens when the parent was deleted but a child somehow
    // survived; treating those as roots keeps the tree view rendering
    // every task instead of silently hiding orphans.
    std::vector<int> rootTaskIds(const data::TaskStore& store) {
        std::unordered_set<int> present;
        present.reserve(store.tasks.size());
        for (const data::Task& t : store.tasks) {
            present.insert(t.id);
        }
        std::vector<int> out;
        out.reserve(store.tasks.size());
        for (const data::Task& t : store.tasks) {
            int pid = t.parentId;
            if (pid == -1 || present.find(pid) == present.end()) {
                out.push_back(t.id);
            }
        }
        return out;
    }

    std::vector<int> childTaskIds(const data::TaskStore& store, int parentId) {
        std::vector<int> out;
        for (const data::Task& t : store.tasks) {
            if (t.parentId == parentId) {
                out.push_back(t.id);
            }
        }
        return out;
    }

    // Validate first, then mutate. Allocating the next id only after
    // validation passes keeps the id sequence dense (no gaps from
    // rejected drafts).
    int createTask(data::TaskStore& store,
                   const data::Task& draft,
                   ValidationResult& outResult) {
        outResult = validateDraftTask(store, draft, false);
        if (!outResult.ok) {
            return -1;
        }
        data::Task t = draft;
        t.id         = data::allocateNextId(store);
        data::Date today = logic::today();
        t.createdAt      = today;
        t.updatedAt      = today;
        data::addTaskToStore(store, t);
        return t.id;
    }

    // Edits preserve createdAt and bump updatedAt. The draft's createdAt
    // is intentionally ignored so the UI cannot rewrite history.
    bool editTask(data::TaskStore& store,
                  const data::Task& updated,
                  ValidationResult& outResult) {
        const data::Task* existing =
            data::findTaskInStoreConst(store, updated.id);
        if (existing == nullptr) {
            outResult.ok      = false;
            outResult.message = "Task not found.";
            return false;
        }
        outResult = validateDraftTask(store, updated, true);
        if (!outResult.ok) {
            return false;
        }
        data::Task copy = updated;
        copy.createdAt  = existing->createdAt;
        copy.updatedAt  = logic::today();
        return data::updateTaskInStore(store, copy);
    }

    // Cascade delete: gather every descendant id first, then remove. We
    // collect before removing because removing during traversal would
    // invalidate the iteration the recursion depends on.
    int deleteTaskCascade(data::TaskStore& store, int rootId) {
        const data::Task* root = data::findTaskInStoreConst(store, rootId);
        if (root == nullptr) {
            return 0;
        }
        std::vector<int> victims;
        victims.push_back(rootId);
        collectDescendantIds(store, rootId, victims);
        int removed = 0;
        for (int victimId : victims) {
            if (data::removeTaskFromStore(store, victimId)) {
                removed += 1;
            }
        }
        return removed;
    }

    // Snapshot/restore back the single-step undo. We deliberately keep
    // the existing filePath on restore so undo doesn't bounce the user
    // back to a different file context.
    void takeSnapshot(const data::TaskStore& store,
                      data::TaskStore& snapshotOut) {
        snapshotOut = store;
    }

    void restoreSnapshot(data::TaskStore& store,
                         const data::TaskStore& snapshot) {
        std::string keepPath = store.filePath;
        store          = snapshot;
        store.filePath = keepPath;
        store.dirty    = true;
    }

}
