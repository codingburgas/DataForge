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

    // A single multiple-choice decision attached to a task. Picking an
    // option flips the task into the option's target status. chosenIndex
    // is -1 until the user picks; once set it stays for audit trail.
    struct DecisionOption {
        std::string text;
        Status      effect;
    };

    struct Decision {
        std::string                 question;
        std::vector<DecisionOption> options;
        int                         chosenIndex = -1;
    };

    // A task is a plain struct with `parentId == -1` meaning "root".
    // Every other field is owned by the task itself; there are no
    // pointers between tasks, so copying a Task is cheap and safe.
    struct Task {
        int                   id;
        int                   parentId;
        std::string           title;
        std::string           description;
        Priority              priority;
        Status                status;
        Date                  deadline;
        int                   estimatedMinutes;
        int                   actualMinutes;
        Date                  createdAt;
        Date                  updatedAt;
        std::vector<Decision> decisions;
    };

    // Action types for the history log. Stored as int in the file so
    // appending new actions later does not break older saves.
    enum HistoryAction {
        HIST_CREATE        = 0,
        HIST_EDIT          = 1,
        HIST_DELETE        = 2,
        HIST_STATUS_CHANGE = 3,
        HIST_LOAD          = 4,
        HIST_SAVE          = 5,
        HIST_DECISION      = 6
    };

    // One entry in the action/changes log. taskId == -1 for store-wide
    // events (load, save). timestamp is local-time ISO datetime.
    struct HistoryEntry {
        int           id          = 0;
        std::string   timestamp;
        HistoryAction action      = HIST_CREATE;
        int           taskId      = -1;
        std::string   summary;
    };

    // Productivity tracking. Persisted with the store so progress
    // survives restart. completionDates feeds the streak calculation.
    struct ProductivityState {
        int                       xp           = 0;
        int                       totalDone    = 0;
        int                       totalCreated = 0;
        int                       totalEdited  = 0;
        std::vector<std::string>  earnedBadges;
        std::vector<std::string>  completionDates;
    };

    // Whole-document container. `dirty` drives the unsaved-changes
    // marker in the title bar; `filePath` is empty when the store has
    // never been saved to disk.
    struct TaskStore {
        std::vector<Task>          tasks;
        int                        nextId;
        std::string                filePath;
        bool                       dirty;
        std::vector<HistoryEntry>  history;
        int                        nextHistoryId;
        ProductivityState          productivity;
    };

}
