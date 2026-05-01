#include "data/store.h"

namespace data {

    TaskStore createEmptyStore() {
        TaskStore store;
        store.tasks.clear();
        store.nextId  = 1;
        store.filePath.clear();
        store.dirty   = false;
        return store;
    }

    void markDirty(TaskStore& store) {
        store.dirty = true;
    }

    int indexOfTask(const TaskStore& store, int id) {
        for (std::size_t i = 0; i < store.tasks.size(); ++i) {
            if (store.tasks[i].id == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

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

    int allocateNextId(TaskStore& store) {
        int id = store.nextId;
        store.nextId += 1;
        store.dirty   = true;
        return id;
    }

}
