# Sort Benchmark Panel

## BENCH-01 — Run with N=1000
- **Preconditions**: Benchmark panel open.
- **Steps**:
  1. Set `N = 1000`.
  2. Click **Run**.
- **Expected**: Two times appear (Bubble vs Quick) in milliseconds. Quick is faster.
- **Priority**: Medium
- **Type**: Performance

## BENCH-02 — Run with N=10000
- **Preconditions**: Benchmark panel open.
- **Steps**:
  1. Set `N = 10000`.
  2. Click **Run**.
- **Expected**: UI stays responsive. Bubble is much slower than Quick.
- **Priority**: Medium
- **Type**: Performance

## BENCH-03 — Run with N=0
- **Steps**: Set `N = 0`, click **Run**.
- **Expected**: Inline error or 0ms result. No crash.
- **Priority**: Low
- **Type**: Negative
