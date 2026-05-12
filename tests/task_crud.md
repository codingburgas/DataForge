# Task CRUD

## CRUD-01 — Create a root task
- **Preconditions**: App open, empty store.
- **Steps**:
  1. Press `Ctrl + N`.
  2. Type title `Buy milk`, priority `Medium`, deadline `+3 days`.
  3. Click **Save**.
- **Expected**: Task appears in the task table. Status bar total = 1. Window title shows unsaved indicator.
- **Priority**: High
- **Type**: Functional

## CRUD-02 — Edit a task title
- **Preconditions**: At least one task exists.
- **Steps**:
  1. Select the task.
  2. Open **Task Details** panel.
  3. Change the title to `Buy oat milk`.
  4. Press **Apply**.
- **Expected**: Table row updates. Dirty flag is set.
- **Priority**: High
- **Type**: Functional

## CRUD-03 — Delete a root task with no children
- **Preconditions**: A leaf task is selected.
- **Steps**:
  1. Press `Delete`.
  2. Confirm in the dialog.
- **Expected**: Task is removed. Status bar total decreases by 1.
- **Priority**: High
- **Type**: Functional

## CRUD-04 — Cancel delete confirmation
- **Preconditions**: A task is selected.
- **Steps**:
  1. Press `Delete`.
  2. Click **Cancel** in the dialog.
- **Expected**: No change to the store. Task still exists.
- **Priority**: Medium
- **Type**: UI / Negative

## CRUD-05 — Create task with empty title
- **Preconditions**: New-task dialog open.
- **Steps**:
  1. Leave title empty.
  2. Click **Save**.
- **Expected**: Inline error appears. Toast says `Title is required`. Store is unchanged.
- **Priority**: High
- **Type**: Negative

## CRUD-06 — Mark task complete
- **Preconditions**: Task with status `Open` exists.
- **Steps**:
  1. Click the status cell.
  2. Pick `Done`.
- **Expected**: Status updates. Stats panel `Done` counter goes up by 1.
- **Priority**: Medium
- **Type**: Functional
