# Status Bar

## SBAR-01 — Total count
- **Steps**: Add and remove tasks.
- **Expected**: Status bar `Total: N` follows the store size at all times.
- **Priority**: Medium
- **Type**: UI

## SBAR-02 — Filtered count
- **Steps**: Apply a filter.
- **Expected**: Shows `Showing X of N`.
- **Priority**: Medium
- **Type**: UI

## SBAR-03 — Last action
- **Steps**: Delete a task.
- **Expected**: Status bar text reads `Removed: <title>` for a few seconds.
- **Priority**: Low
- **Type**: UI

## SBAR-04 — Save state indicator
- **Steps**: Make a change, save.
- **Expected**: Indicator goes from `Modified` to `Saved`.
- **Priority**: Medium
- **Type**: UI
