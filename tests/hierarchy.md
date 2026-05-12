# Hierarchy & Subtasks

## HIER-01 — Add a subtask
- **Preconditions**: One root task exists.
- **Steps**:
  1. Select the root task.
  2. Click **Add Subtask**.
  3. Save with title `Step 1`.
- **Expected**: New task has the root as its `parentId`. Task tree shows it nested one level deep.
- **Priority**: High
- **Type**: Functional

## HIER-02 — Deep nesting (5 levels)
- **Preconditions**: Empty store.
- **Steps**:
  1. Create root.
  2. Add subtask, then subtask of that, repeat to 5 levels deep.
- **Expected**: Tree renders all 5 levels. Recursion-based max-depth function reports 5.
- **Priority**: Medium
- **Type**: Functional

## HIER-03 — Cascade delete
- **Preconditions**: Root task with 3 subtasks and 1 grandchild.
- **Steps**:
  1. Select the root.
  2. Press `Delete`.
  3. Read the confirmation. It should say `4 descendants will be removed`.
  4. Confirm.
- **Expected**: Root and all 4 descendants are gone. Store total drops by 5.
- **Priority**: High
- **Type**: Functional

## HIER-04 — Estimated time sums up the tree
- **Preconditions**: Root with three children, each 1 hour estimate.
- **Steps**:
  1. Look at the root row total-time column.
- **Expected**: Total reads `3h` even though the root itself has no own estimate.
- **Priority**: Medium
- **Type**: Functional

## HIER-05 — Weighted completion
- **Preconditions**: Root with two children. One child is `Done`, the other is `Open`.
- **Steps**:
  1. Read the root completion percentage.
- **Expected**: Shows `50%`.
- **Priority**: Medium
- **Type**: Functional

## HIER-06 — Subtask of a deleted parent
- **Preconditions**: Try a race: delete a parent while editing a subtask.
- **Steps**:
  1. Open subtask details.
  2. Delete the parent in the table.
- **Expected**: Subtask editor closes safely. No crash. Toast warns the parent was removed.
- **Priority**: Low
- **Type**: Negative
