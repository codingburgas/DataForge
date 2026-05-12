# Theme

## THEME-01 — Switch dark to light
- **Preconditions**: App in dark mode.
- **Steps**:
  1. Open **View → Theme → Light**.
- **Expected**: UI repaints in light colors right away. No flicker.
- **Priority**: Medium
- **Type**: UI

## THEME-02 — Theme persists across runs
- **Preconditions**: User picked light mode.
- **Steps**:
  1. Close the app.
  2. Reopen.
- **Expected**: Starts in light mode.
- **Priority**: Medium
- **Type**: Regression

## THEME-03 — Productivity tab refresh under theme change
- **Preconditions**: Productivity tab open.
- **Steps**:
  1. Switch theme.
- **Expected**: Charts re-render with the new palette. No stale colors. (Covers the fix in commit `3f51c39`.)
- **Priority**: High
- **Type**: Regression
