#pragma once

#include "libraries.h"
#include "data/types.h"

namespace logic {

    struct ValidationResult {
        bool        ok;
        std::string message;
    };

    ValidationResult validateDraftTask(const data::TaskStore& store,
                                       const data::Task& draft,
                                       bool isEdit);

    bool wouldExceedDepth(const data::TaskStore& store,
                          int parentId,
                          int maxDepth);

    bool createsCycle(const data::TaskStore& store,
                      int taskId,
                      int candidateParentId);

}
