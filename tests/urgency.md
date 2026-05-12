# Urgency Coloring

## URG-01 — Overdue tint
- **Preconditions**: Task with deadline 2 days ago, status `Open`.
- **Expected**: Deadline cell is red.
- **Priority**: High
- **Type**: UI

## URG-02 — Soon tint
- **Preconditions**: Task with deadline in 2 days.
- **Expected**: Deadline cell is amber.
- **Priority**: Medium
- **Type**: UI

## URG-03 — Comfort tint
- **Preconditions**: Task with deadline in 30 days.
- **Expected**: Deadline cell is green.
- **Priority**: Low
- **Type**: UI

## URG-04 — Done overrides red
- **Preconditions**: Overdue task marked `Done`.
- **Expected**: Red tint is dropped. Cell goes neutral.
- **Priority**: Medium
- **Type**: UI
