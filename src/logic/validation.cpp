#include "logic/validation.h"
#include "logic/tasks.h"
#include "data/store.h"
#include "data/date.h"

namespace logic {

    // Trim all leading/trailing ASCII whitespace. Done by hand so the
    // validation layer has no dependency on locale or <algorithm>.
    static std::string trim(const std::string& s) {
        std::size_t start = 0;
        std::size_t end   = s.size();
        while (start < end) {
            char c = s[start];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                start += 1;
            } else {
                break;
            }
        }
        while (end > start) {
            char c = s[end - 1];
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                end -= 1;
            } else {
                break;
            }
        }
        return s.substr(start, end - start);
    }

    // Walks parent chain and returns true if attaching a new child under
    // `parentId` would push the subtree past `maxDepth`. Stops as soon as
    // the budget is blown so we never traverse a corrupted infinite chain.
    bool wouldExceedDepth(const data::TaskStore& store,
                          int parentId,
                          int maxDepth) {
        int depth = 0;
        int cur   = parentId;
        while (cur != -1) {
            depth += 1;
            if (depth > maxDepth) {
                return true;
            }
            const data::Task* p = data::findTaskInStoreConst(store, cur);
            if (p == nullptr) {
                return false;
            }
            cur = p->parentId;
        }
        return false;
    }

    // Cycle detection: walks up from the candidate parent and returns true
    // if we ever reach `taskId` (which would mean the task is being made
    // its own ancestor). Bounded by MAX_TREE_DEPTH+1 hops so a pre-existing
    // cycle in the store cannot lock the editor up.
    bool createsCycle(const data::TaskStore& store,
                      int taskId,
                      int candidateParentId) {
        if (candidateParentId == -1) {
            return false;
        }
        if (candidateParentId == taskId) {
            return true;
        }
        int cur  = candidateParentId;
        int hops = 0;
        while (cur != -1 && hops < MAX_TREE_DEPTH + 1) {
            if (cur == taskId) {
                return true;
            }
            const data::Task* p = data::findTaskInStoreConst(store, cur);
            if (p == nullptr) {
                return false;
            }
            cur   = p->parentId;
            hops += 1;
        }
        // Hit the hop limit without resolving — treat as a cycle so the
        // edit is rejected rather than silently accepted.
        return hops >= MAX_TREE_DEPTH + 1;
    }

    // Single entry point used for both create and edit. `isEdit` flips
    // on the cycle check, since cycles are only meaningful when the task
    // already exists in the store.
    ValidationResult validateDraftTask(const data::TaskStore& store,
                                       const data::Task& draft,
                                       bool isEdit) {
        ValidationResult r;
        r.ok = false;

        // Title is the only required field. Trim first so a string of
        // whitespace doesn't masquerade as a real title.
        std::string title = trim(draft.title);
        if (title.empty()) {
            r.message = "Title is required.";
            return r;
        }
        if (title.size() > 200) {
            r.message = "Title must be at most 200 characters.";
            return r;
        }
        if (draft.description.size() > 2000) {
            r.message = "Description must be at most 2000 characters.";
            return r;
        }
        // Enum range checks defend against malformed .dftasks input where
        // someone hand-edited the priority/status integer.
        if (draft.priority < data::PRIORITY_LOW ||
            draft.priority > data::PRIORITY_CRITICAL) {
            r.message = "Priority is out of range.";
            return r;
        }
        if (draft.status < data::STATUS_TODO ||
            draft.status > data::STATUS_BLOCKED) {
            r.message = "Status is out of range.";
            return r;
        }
        if (!data::isDateValid(draft.deadline)) {
            r.message = "Deadline is not a valid date.";
            return r;
        }
        // Cap minutes at one year so a typo (e.g. 9999999) cannot make
        // the stats panel render absurd totals.
        if (draft.estimatedMinutes < 0 || draft.estimatedMinutes > 24 * 60 * 365) {
            r.message = "Estimated minutes is out of range.";
            return r;
        }
        if (draft.actualMinutes < 0 || draft.actualMinutes > 24 * 60 * 365) {
            r.message = "Actual minutes is out of range.";
            return r;
        }
        if (draft.parentId != -1) {
            const data::Task* parent =
                data::findTaskInStoreConst(store, draft.parentId);
            if (parent == nullptr) {
                r.message = "Parent task does not exist.";
                return r;
            }
            if (isEdit && createsCycle(store, draft.id, draft.parentId)) {
                r.message = "Parent assignment would create a cycle.";
                return r;
            }
            // -1 because attaching a new child adds one more level beneath
            // whatever depth the parent already sits at.
            if (wouldExceedDepth(store, draft.parentId, MAX_TREE_DEPTH - 1)) {
                r.message = "Adding here would exceed the maximum tree depth.";
                return r;
            }
        }
        r.ok      = true;
        r.message = std::string();
        return r;
    }

}
