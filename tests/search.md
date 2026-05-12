# Search

## SRCH-01 — Substring search in titles
- **Preconditions**: Tasks include `Buy milk`, `Buy bread`, `Pay rent`.
- **Steps**:
  1. Press `Ctrl + F`.
  2. Type `buy`.
- **Expected**: Two rows show. Case does not matter.
- **Priority**: High
- **Type**: Functional

## SRCH-02 — Empty query restores list
- **Preconditions**: A filter from SRCH-01 is active.
- **Steps**:
  1. Clear the search box.
- **Expected**: All tasks visible again.
- **Priority**: Medium
- **Type**: Functional

## SRCH-03 — No match
- **Preconditions**: Store has tasks but none match `xyzzy`.
- **Steps**:
  1. Type `xyzzy` in search.
- **Expected**: Table is empty. Status bar shows `0 of N`.
- **Priority**: Medium
- **Type**: Functional

## SRCH-04 — Binary search by ID hit
- **Preconditions**: A task with known ID `42` exists.
- **Steps**:
  1. Open dev quick-jump or the binary-search entry point.
  2. Enter ID `42`.
- **Expected**: Selection jumps to task `42` in O(log N) lookups.
- **Priority**: Medium
- **Type**: Functional

## SRCH-05 — Binary search by ID miss
- **Preconditions**: No task with ID `9999`.
- **Steps**:
  1. Enter ID `9999`.
- **Expected**: Toast `Task not found`. Selection does not change.
- **Priority**: Low
- **Type**: Negative
