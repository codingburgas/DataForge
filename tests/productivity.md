# Productivity Dashboard

## PROD-01 — Dashboard opens
- **Steps**: Open the **Productivity** tab.
- **Expected**: Panel renders. Pie chart visible. No empty white space.
- **Priority**: High
- **Type**: UI

## PROD-02 — Pie chart legend
- **Steps**: Hover each slice.
- **Expected**: Legend shows label and count for each slice. (Covers commit `e5a7a0e`.)
- **Priority**: Medium
- **Type**: UI

## PROD-03 — History tracking
- **Steps**: Complete two tasks, reopen the dashboard.
- **Expected**: History list shows both events with timestamp.
- **Priority**: Medium
- **Type**: Functional

## PROD-04 — Decision tree node
- **Steps**: Use the decision tree feature on a complex task.
- **Expected**: Tree renders branches. Click expands and collapses nodes.
- **Priority**: Medium
- **Type**: Functional

## PROD-05 — Empty store dashboard
- **Preconditions**: No tasks at all.
- **Steps**: Open dashboard.
- **Expected**: Friendly message `No data yet`. No divide-by-zero.
- **Priority**: Low
- **Type**: Negative
