#pragma once

#include "libraries.h"
#include "data/types.h"

// Single source of truth for "is this draft task acceptable?". The UI
// layer never invents its own validation rules — it always calls in here.
namespace logic {

    // ok=true means the draft passed all checks. When ok=false, message
    // holds a user-facing reason (already localised at presentation
    // time via the i18n table).
    struct ValidationResult {
        bool        ok;
        std::string message;
    };

    // Validate a task before it is added to or replaces an entry in the
    // store. `isEdit` enables the cycle check (only meaningful when the
    // task already exists).
    ValidationResult validateDraftTask(const data::TaskStore& store,
                                       const data::Task& draft,
                                       bool isEdit);

    // True iff attaching a new child under `parentId` would push the
    // subtree past `maxDepth`.
    bool wouldExceedDepth(const data::TaskStore& store,
                          int parentId,
                          int maxDepth);

    // True iff making `candidateParentId` the parent of `taskId` would
    // form a cycle in the parent chain.
    bool createsCycle(const data::TaskStore& store,
                      int taskId,
                      int candidateParentId);

}
