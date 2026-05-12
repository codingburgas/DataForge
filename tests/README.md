# DataForge QA Test Cases

Manual QA test cases for the DataForge task manager. Each file groups cases by feature area.

## Layout

| File | Scope |
|---|---|
| `task_crud.md` | Create, read, update, delete a task |
| `hierarchy.md` | Subtasks, parent/child links, cascade delete |
| `sort.md` | Bubble sort, Quick sort, sort key selection |
| `search.md` | Substring linear search, recursive binary search by ID |
| `filter.md` | Status and priority filters, compose with search |
| `statistics.md` | Stats panel counts and totals |
| `persistence.md` | Save, load, atomic write-and-rename, dirty flag |
| `validation.md` | Input rules, inline errors, toast errors |
| `undo.md` | Single-step undo after destructive ops |
| `shortcuts.md` | Keyboard shortcuts |
| `theme.md` | Light / Dark theme switch and persistence |
| `benchmark.md` | Sort benchmark panel timings |
| `urgency.md` | Deadline urgency coloring |
| `status_bar.md` | Status bar counts and labels |
| `productivity.md` | Productivity dashboard, history, pie chart |
| `voice.md` | Voice input panel |
| `i18n.md` | Language switch |

## Case format

Every case follows the same shape:

- **ID** — area code + number, e.g. `CRUD-01`
- **Title** — short one-line goal
- **Preconditions** — state needed before test
- **Steps** — numbered actions
- **Expected** — what the app must do
- **Priority** — High / Medium / Low
- **Type** — Functional / UI / Negative / Performance / Regression

## Pass criteria

A case passes when every step produces the expected result with no crash, no console error, and no leaked memory in the debug heap.
