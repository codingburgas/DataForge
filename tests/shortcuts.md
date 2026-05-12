# Keyboard Shortcuts

## KEY-01 — Ctrl+N new task
- **Steps**: Press `Ctrl + N` from any panel.
- **Expected**: New-task dialog opens with focus on the title field.
- **Priority**: High
- **Type**: Functional

## KEY-02 — Ctrl+S save
- **Steps**: Press `Ctrl + S` on a dirty store.
- **Expected**: Save dialog or quick save runs. Dirty flag clears.
- **Priority**: High
- **Type**: Functional

## KEY-03 — Ctrl+O open
- **Steps**: Press `Ctrl + O`.
- **Expected**: File-open dialog appears.
- **Priority**: High
- **Type**: Functional

## KEY-04 — Ctrl+F focus search
- **Steps**: Press `Ctrl + F`.
- **Expected**: Search box gets focus. Caret blinking inside.
- **Priority**: Medium
- **Type**: UI

## KEY-05 — Ctrl+Z undo
- **Steps**: Delete a task, press `Ctrl + Z`.
- **Expected**: See UNDO-01.
- **Priority**: High
- **Type**: Functional

## KEY-06 — Delete key
- **Steps**: Select a task, press `Delete`.
- **Expected**: Delete confirmation appears.
- **Priority**: High
- **Type**: Functional

## KEY-07 — Arrow keys
- **Steps**: Use `↑` and `↓` in the task table.
- **Expected**: Selection moves one row at a time. No scroll glitch.
- **Priority**: Medium
- **Type**: UI

## KEY-08 — Shortcut while dialog open
- **Steps**: Open a modal dialog, press `Ctrl + N`.
- **Expected**: Shortcut is ignored. Dialog stays focused. No second dialog.
- **Priority**: Medium
- **Type**: Negative
