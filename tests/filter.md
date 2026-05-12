# Filter

## FILT-01 — Filter by status `Done`
- **Preconditions**: Mixed-status tasks.
- **Steps**:
  1. Open the filter menu.
  2. Pick **Status: Done**.
- **Expected**: Only `Done` tasks show. Status bar updates filtered count.
- **Priority**: High
- **Type**: Functional

## FILT-02 — Filter by priority `High`
- **Preconditions**: Mixed-priority tasks.
- **Steps**:
  1. Pick **Priority: High**.
- **Expected**: Only `High` priority rows show.
- **Priority**: High
- **Type**: Functional

## FILT-03 — Filter plus search compose
- **Preconditions**: Five `High` tasks, two contain `report`.
- **Steps**:
  1. Filter **Priority: High**.
  2. Search `report`.
- **Expected**: Two rows. Both filter and search apply at once.
- **Priority**: Medium
- **Type**: Functional

## FILT-04 — Clear filter
- **Preconditions**: A filter is active.
- **Steps**:
  1. Click **Clear filter**.
- **Expected**: All rows return. Filtered count equals total.
- **Priority**: Medium
- **Type**: Functional
