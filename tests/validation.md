# Validation

## VAL-01 — Title length cap
- **Preconditions**: New-task dialog open.
- **Steps**:
  1. Paste a 500-character title.
  2. Save.
- **Expected**: Save blocked. Inline error `Title too long (max N)`.
- **Priority**: Medium
- **Type**: Negative

## VAL-02 — Deadline in the past on create
- **Preconditions**: New-task dialog open.
- **Steps**:
  1. Set deadline to yesterday.
  2. Save.
- **Expected**: Save allowed but row deadline cell shows red overdue color at once.
- **Priority**: Low
- **Type**: Functional

## VAL-03 — Negative estimated time
- **Preconditions**: New-task dialog open.
- **Steps**:
  1. Set estimate to `-1`.
- **Expected**: Inline error. Save blocked.
- **Priority**: Medium
- **Type**: Negative

## VAL-04 — Invalid date format
- **Preconditions**: New-task dialog open.
- **Steps**:
  1. Type `2026-13-40` in the deadline field.
- **Expected**: Inline error `Invalid date`. Save blocked.
- **Priority**: Medium
- **Type**: Negative
