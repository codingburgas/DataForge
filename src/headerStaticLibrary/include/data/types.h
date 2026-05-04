#pragma once

#include "libraries.h"

// Plain-old-data types shared across all three layers. Kept free of any
// behaviour so each layer can include this without dragging logic in.
namespace data {

    // Higher numeric values mean more urgent. The order matters: bubble
    // sort uses `>` on the underlying int to put Critical first.
    enum Priority {
        PRIORITY_LOW      = 0,
        PRIORITY_MEDIUM   = 1,
        PRIORITY_HIGH     = 2,
        PRIORITY_CRITICAL = 3
    };

    // Lifecycle of a single task. STATUS_BLOCKED is the highest valid
    // value; the validator clamps incoming values to [TODO, BLOCKED].
    enum Status {
        STATUS_TODO        = 0,
        STATUS_IN_PROGRESS = 1,
        STATUS_DONE        = 2,
        STATUS_BLOCKED     = 3
    };

    // Calendar date. {0,0,0} is the sentinel for "no date set".
    struct Date {
        int year;
        int month;
        int day;
    };

    // A task is a plain struct with `parentId == -1` meaning "root".
    // Every other field is owned by the task itself; there are no
    // pointers between tasks, so copying a Task is cheap and safe.
    struct Task {
        int         id;
        int         parentId;
        std::string title;
        std::string description;
        Priority    priority;
        Status      status;
        Date        deadline;
        int         estimatedMinutes;
        int         actualMinutes;
        Date        createdAt;
        Date        updatedAt;
    };

    // Whole-document container. `dirty` drives the unsaved-changes
    // marker in the title bar; `filePath` is empty when the store has
    // never been saved to disk.
    struct TaskStore {
        std::vector<Task> tasks;
        int               nextId;
        std::string       filePath;
        bool              dirty;
    };

}
