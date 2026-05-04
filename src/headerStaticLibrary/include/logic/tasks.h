#pragma once

#include "libraries.h"
#include "data/types.h"
#include "logic/validation.h"

// High-level task operations the UI is allowed to call. Anything that
// mutates the store goes through one of these so validation, dirty-flag
// management, and timestamp bookkeeping stay in one place.
namespace logic {

    // Hard cap on parent-chain length. Both the validator and every
    // recursive walker honour it as a safety net against corrupted input.
    constexpr int MAX_TREE_DEPTH = 64;

    // Build a fresh empty store (used by File > New and on first run).
    data::TaskStore createEmptyStore();

    // Validate then insert. Returns the new id on success, -1 on
    // validation failure (with details in outResult).
    int  createTask(data::TaskStore& store,
                    const data::Task& draft,
                    ValidationResult& outResult);

    // Validate then replace the existing task with the same id.
    // Preserves the original createdAt; bumps updatedAt to today.
    bool editTask(data::TaskStore& store,
                  const data::Task& updated,
                  ValidationResult& outResult);

    // Remove a task and its entire subtree. Returns the number of
    // tasks actually removed.
    int  deleteTaskCascade(data::TaskStore& store, int rootId);

    // Tree-shape queries used by the table and the tree view.
    std::vector<int> rootTaskIds(const data::TaskStore& store);
    std::vector<int> childTaskIds(const data::TaskStore& store, int parentId);

    // Snapshot / restore that powers single-step undo. The snapshot is
    // a full copy of the store; the restore preserves the current
    // filePath so undo never bounces the user to a different file.
    void takeSnapshot(const data::TaskStore& store,
                      data::TaskStore& snapshotOut);
    void restoreSnapshot(data::TaskStore& store,
                         const data::TaskStore& snapshot);

}
