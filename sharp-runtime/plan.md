# plan.md — sharp-runtime planning index
*Last updated: 2026-07-07 — 10711 tests passing*

sharp-runtime is a C++23 static library reimplementing a practical subset of .NET `System.*` for **CNA** (C++ XNA port) and **mobile-eggbert** (ported Windows Phone game).

Reference source: `/rv/tmp/runtime/src/libraries/` (dotnet/runtime, MIT License)

---

## Planning is now database-driven

This file and [plan_namespaces.md](plan_namespaces.md) are **historical** — the interactive,
per-namespace-table workflow they describe has been replaced by `plan.sqlite3`, a git-ignored
SQLite database that is the live, authoritative source of truth. See `README.md`'s "Tracking:
plan.sqlite3" section for the full explanation of its two tables (`task` for .NET type
classification, `ticket` for stabilization work).

```bash
# Namespace-level porting status
sqlite3 plan.sqlite3 "SELECT namespace, status, COUNT(*) FROM task GROUP BY namespace, status ORDER BY namespace;"

# Overall status counts
sqlite3 plan.sqlite3 "SELECT status, COUNT(*) FROM task GROUP BY status ORDER BY COUNT(*) DESC;"

# Stabilization ticket queue
sqlite3 plan.sqlite3 "SELECT status, priority, COUNT(*) FROM ticket GROUP BY status, priority ORDER BY priority, status;"
```

`plan_files.md`, referenced by an earlier version of this file, was never created and does not exist.

## Legend (task.status — see CLAUDE.md and README.md for the full definitions)

| Status | Meaning |
|--------|---------|
| `ported` | Implemented in sharp-runtime, meets the full porting checklist |
| `todo` / `''` | Needs to be ported/classified |
| `ignore` / `ignored` | Out of scope for sharp-runtime |
| `tobedecided` | Genuinely ambiguous — needs a human architecture decision |

`in_progress` is **not** a valid status value in the current workflow (it was used by an older,
now-retired interactive process — see `plan_namespaces.md`).
