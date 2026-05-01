#pragma once

#include "libraries.h"

namespace data {

    enum Priority {
        PRIORITY_LOW      = 0,
        PRIORITY_MEDIUM   = 1,
        PRIORITY_HIGH     = 2,
        PRIORITY_CRITICAL = 3
    };

    enum Status {
        STATUS_TODO        = 0,
        STATUS_IN_PROGRESS = 1,
        STATUS_DONE        = 2,
        STATUS_BLOCKED     = 3
    };

    struct Date {
        int year;
        int month;
        int day;
    };

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

    struct TaskStore {
        std::vector<Task> tasks;
        int               nextId;
        std::string       filePath;
        bool              dirty;
    };

}
