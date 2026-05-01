#pragma once

#include "libraries.h"
#include "data/types.h"

namespace data {

    TaskStore   createEmptyStore();
    int         addTaskToStore(TaskStore& store, const Task& task);
    bool        removeTaskFromStore(TaskStore& store, int id);
    Task*       findTaskInStore(TaskStore& store, int id);
    const Task* findTaskInStoreConst(const TaskStore& store, int id);
    bool        updateTaskInStore(TaskStore& store, const Task& updated);
    int         indexOfTask(const TaskStore& store, int id);
    int         allocateNextId(TaskStore& store);
    void        markDirty(TaskStore& store);

}
