#include "data/store.h"

namespace data {

    // Factory used by both the "File > New" path and the load path before
    // a successful parse — guarantees every field has a sane default.
    TaskStore createEmptyStore() {
        TaskStore store;
        store.tasks.clear();
        store.nextId        = 1;
        store.filePath.clear();
        store.dirty         = false;
        store.history.clear();
        store.nextHistoryId = 1;
        store.productivity  = ProductivityState{};
        return store;
    }

    void markDirty(TaskStore& store) {
        store.dirty = true;
    }

    // Linear scan: the store is small enough (hundreds, not millions) that
    // an unordered_map index would cost more in cache misses and code
    // complexity than the O(n) scan saves. Returns -1 when not found so
    // callers can do a simple `< 0` check.
    int indexOfTask(const TaskStore& store, int id) {
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // Mutating ops all set dirty=true so the title bar's unsaved-marker
    // stays in sync without the caller having to remember.
    int addTaskToStore(TaskStore& store, const Task& task) {
        store.tasks.push_back(task);
        store.dirty = true;
        return task.id;
    }

    bool removeTaskFromStore(TaskStore& store, int id) {
        int idx = indexOfTask(store, id);
        if (idx < 0) {
            return false;
        }
        store.tasks.erase(store.tasks.begin() + idx);
        store.dirty = true;
        return true;
    }

    Task* findTaskInStore(TaskStore& store, int id) {
        int idx = indexOfTask(store, id);
        if (idx < 0) {
            return nullptr;
        }
        return &store.tasks[idx];
    }

    // const overload so read-only paths (validation, recursion) can ask
    // for a task without giving up immutability of the store.
    const Task* findTaskInStoreConst(const TaskStore& store, int id) {
        int idx = indexOfTask(store, id);
        if (idx < 0) {
            return nullptr;
        }
        return &store.tasks[idx];
    }

    bool updateTaskInStore(TaskStore& store, const Task& updated) {
        int idx = indexOfTask(store, updated.id);
        if (idx < 0) {
            return false;
        }
        store.tasks[idx] = updated;
        store.dirty      = true;
        return true;
    }

    // Monotonic id allocator. Never reuses ids even after delete so that
    // any cached references (UI selection, undo snapshot) cannot resolve
    // to a different task than the one they originally pointed at.
    int allocateNextId(TaskStore& store) {
        int id = store.nextId;
        store.nextId += 1;
        store.dirty   = true;
        return id;
    }

}
