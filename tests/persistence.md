# Persistence

## PERS-01 — Save a fresh store
- **Preconditions**: New store with three tasks. No file yet.
- **Steps**:
  1. Press `Ctrl + S`.
  2. Pick a path `my.dftasks`.
- **Expected**: File written. Dirty flag clears. Title bar drops the unsaved marker.
- **Priority**: High
- **Type**: Functional

## PERS-02 — Load an existing file
- **Preconditions**: A valid `.dftasks` file on disk.
- **Steps**:
  1. Press `Ctrl + O`.
  2. Pick the file.
- **Expected**: Tasks load. Row count matches file. No parse errors.
- **Priority**: High
- **Type**: Functional

## PERS-03 — Atomic save survives a crash mid-write
- **Preconditions**: A working file with 100 tasks.
- **Steps**:
  1. Save normally once.
  2. Use a debugger to break right after the temp file is written but before rename.
  3. Kill the app.
  4. Reopen the app and load the file.
- **Expected**: Old file is intact. No corruption. The temp file may linger and is harmless.
- **Priority**: High
- **Type**: Negative

## PERS-04 — Open a corrupt file
- **Preconditions**: A `.dftasks` file with a garbled line.
- **Steps**:
  1. Try to open it.
- **Expected**: Toast `Could not parse file at line N`. Store stays as it was.
- **Priority**: Medium
- **Type**: Negative

## PERS-05 — Exit with unsaved changes
- **Preconditions**: Dirty store.
- **Steps**:
  1. Click the window close button.
- **Expected**: Dialog asks **Save / Discard / Cancel**. Choosing **Cancel** keeps the app open.
- **Priority**: High
- **Type**: Functional
