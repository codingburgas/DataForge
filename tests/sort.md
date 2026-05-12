# Sort

## SORT-01 — Bubble sort by priority descending
- **Preconditions**: 5 tasks with mixed priorities.
- **Steps**:
  1. Open the sort menu.
  2. Pick **Bubble Sort — Priority DESC**.
- **Expected**: Rows reorder so `High` tasks come first, then `Medium`, then `Low`. Equal priorities keep original order.
- **Priority**: High
- **Type**: Functional

## SORT-02 — Quick sort by deadline ascending
- **Preconditions**: 5 tasks with mixed deadlines, one with no deadline.
- **Steps**:
  1. Open the sort menu.
  2. Pick **Quick Sort — Deadline ASC**.
- **Expected**: Earliest deadline first. Tasks with no deadline sink to the bottom.
- **Priority**: High
- **Type**: Functional

## SORT-03 — Sort an empty store
- **Preconditions**: No tasks.
- **Steps**:
  1. Pick any sort option.
- **Expected**: No crash. Table stays empty.
- **Priority**: Low
- **Type**: Negative

## SORT-04 — Sort one task
- **Preconditions**: One task.
- **Steps**:
  1. Run both sorts.
- **Expected**: Task stays as-is. No crash.
- **Priority**: Low
- **Type**: Negative

## SORT-05 — Sort keeps subtasks under parents
- **Preconditions**: Root with two subtasks.
- **Steps**:
  1. Run a sort.
- **Expected**: Subtasks stay grouped under the root, not scattered across the table.
- **Priority**: Medium
- **Type**: Functional
