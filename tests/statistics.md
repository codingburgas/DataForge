# Statistics

## STAT-01 — Counts by priority
- **Preconditions**: 2 High, 3 Medium, 1 Low.
- **Steps**:
  1. Open the **Stats** panel.
- **Expected**: Reads `High: 2`, `Medium: 3`, `Low: 1`.
- **Priority**: High
- **Type**: Functional

## STAT-02 — Counts by status
- **Preconditions**: 1 Open, 1 In Progress, 2 Done.
- **Steps**:
  1. Open the **Stats** panel.
- **Expected**: Reads each count correctly.
- **Priority**: High
- **Type**: Functional

## STAT-03 — Overdue count
- **Preconditions**: 2 tasks with deadlines in the past, not `Done`.
- **Steps**:
  1. Open the **Stats** panel.
- **Expected**: `Overdue: 2`.
- **Priority**: High
- **Type**: Functional

## STAT-04 — Total estimated time
- **Preconditions**: Three roots, total estimate 7h.
- **Steps**:
  1. Open the **Stats** panel.
- **Expected**: `Total estimated: 7h`.
- **Priority**: Medium
- **Type**: Functional

## STAT-05 — Stats update on edit
- **Preconditions**: Stats panel open.
- **Steps**:
  1. Change a task from `Open` to `Done`.
- **Expected**: Stats counters update at once. No reload needed.
- **Priority**: Medium
- **Type**: Regression
