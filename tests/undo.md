# Undo

## UNDO-01 — Undo a delete
- **Preconditions**: A task is selected and deleted.
- **Steps**:
  1. Press `Ctrl + Z` right after delete.
- **Expected**: Task is restored exactly. Children too.
- **Priority**: High
- **Type**: Functional

## UNDO-02 — Undo limited to one step
- **Preconditions**: Two destructive ops in a row.
- **Steps**:
  1. Delete task A.
  2. Delete task B.
  3. Press `Ctrl + Z`.
  4. Press `Ctrl + Z` again.
- **Expected**: First undo restores B. Second undo does nothing. Toast `Nothing to undo`.
- **Priority**: Medium
- **Type**: Functional

## UNDO-03 — Undo after save
- **Preconditions**: Delete then save.
- **Steps**:
  1. Press `Ctrl + Z`.
- **Expected**: In-memory restore works. Save state is now dirty again.
- **Priority**: Medium
- **Type**: Functional
