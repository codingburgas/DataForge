#pragma once

#include "libraries.h"
#include "data/types.h"
#include "logic/validation.h"

namespace logic {

    constexpr int MAX_TREE_DEPTH = 64;

    data::TaskStore createEmptyStore();

    int  createTask(data::TaskStore& store,
                    const data::Task& draft,
                    ValidationResult& outResult);

    bool editTask(data::TaskStore& store,
                  const data::Task& updated,
                  ValidationResult& outResult);

    int  deleteTaskCascade(data::TaskStore& store, int rootId);

    std::vector<int> rootTaskIds(const data::TaskStore& store);
    std::vector<int> childTaskIds(const data::TaskStore& store, int parentId);

    void takeSnapshot(const data::TaskStore& store,
                      data::TaskStore& snapshotOut);
    void restoreSnapshot(data::TaskStore& store,
                         const data::TaskStore& snapshot);

}
