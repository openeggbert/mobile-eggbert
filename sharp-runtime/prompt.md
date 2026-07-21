# Instructions for autonomous review of plan.sqlite3

## Initialization (at the start of each new context)

1. Read `CLAUDE.md` and `NEXT.md`.
2. Open `plan.sqlite3` — table `task` with columns: `id, namespace, name, type, internal, outofscope, status`.

**All progress state lives in `plan.sqlite3` and git history — never in conversation/chat memory.**
Conversation context gets summarized or reset (compaction, a new session, a crashed process); the
database and the commit log do not. Concretely: every decision, finding, or partial result that
matters beyond the current turn must be written to `plan.sqlite3` (`task.status`/`ticket.notes`) or
committed to git *before* moving on — not left implicit in something said earlier in the chat. If a
fact is only "known" because it was mentioned in conversation and never persisted, treat it as not
known at all after a reset. This file is safe to re-read from a completely fresh context after any
compaction/reset — resume by re-running Step 1, no conversation memory required.

## Workflow — one iteration (fully autonomous, no per-item confirmation)

### Step 1 — Select the next item

- Take the first record where `status = ''` or `status = 'todo'` (skip `ignore`, `ported`, and `tobedecided`).
- **Priority:** namespaces starting with `System` take precedence over others.

### Step 2 — Decide (no user confirmation)

Look **only** in `/rv/tmp/runtime/src/libraries/` for what the type does, then classify it yourself:

- **Clearly worth porting** (ordinary data/logic type, no hard blocker) → go to Step 3.
- **Clearly out of scope** — reflection, IL emit/codegen, GC internals, P/Invoke/interop, serialization
  infrastructure, or anything else `CLAUDE.md`'s "Known permanent deviations" section already names →
  `status = 'ignore'`, `outofscope = 1`.
- **Clearly irrelevant/duplicate/not applicable** to a C++ game-code runtime, but not one of the
  permanent-deviation categories → `status = 'ignore'`, `outofscope = 0`.
- **Genuinely ambiguous** (you cannot confidently classify it without human judgment) →
  `status = 'tobedecided'`. Do not guess, do not port. Move on to the next item.

Do not stop to ask the user which bucket an item belongs in — make the call and proceed.

### Step 3 — Port (only when Step 2 decided "port")

- Check whether the file already exists in sharp-runtime:
  - **Exists** → review against the full checklist in `CLAUDE.md` (API surface, doc-comments, SPDX,
    logic parity with .NET, build, tests) as if it were new. Fix any gaps found — do not rubber-stamp.
  - **Does not exist** → implement it per the `CLAUDE.md` checklist.
- Build clean (`cmake --build build --parallel 4`, zero errors/warnings) and all tests passing
  (`./build/SharpRuntimeTests`) before moving on.
- Set `status = 'ported'`.

### Step 4 — Save to DB, commit, and continue

```sql
UPDATE task SET status = '...', outofscope = ..., updated_at = datetime('now') WHERE id = ...;
```

After porting a type (`status = 'ported'`), create a new git commit containing only the files for
that port — header, any `.cpp` body, and tests. Use `git -c commit.gpgsign=false commit` (GPG signing
times out in this environment). Pushing to `origin/feature/work` is fine as routine work lands there.
**Never push to `develop` or `master`**, and never merge `feature/work` into `develop`, without the
user explicitly asking in that turn.

For `ignore` / `tobedecided` decisions, updating `plan.sqlite3` is enough — no commit needed
(`plan.sqlite3` is gitignored).

Then loop back to Step 1 for the next item. **Do not stop between items to ask the user — keep going.**

## Allowed `status` values

| Value | Meaning |
|-------|---------|
| `''` | Not yet decided |
| `todo` | Will be ported |
| `ported` | Done, satisfies the checklist |
| `ignore` | Skip (out of scope or irrelevant) |
| `tobedecided` | Ambiguous — deferred for the user to decide by hand later |

> `in_progress` **does not exist** — porting happens directly, with no intermediate state.

## When to still stop and ask

Only interrupt the autonomous flow for things no reasonable heuristic can resolve safely: a build
that won't go green after genuine effort, a destructive/irreversible action, or an explicit user
instruction that conflicts with this file. Otherwise keep processing the queue.

---

## Stabilization work — the `ticket` table

Once `task` has no `todo`/`''` rows left (mechanical porting queue exhausted), stabilization work
is tracked separately in `plan.sqlite3`'s `ticket` table — **do not confuse it with `task`.**
`ticket.status` values are `todo`/`doing`/`done`/`blocked`/`needs_user`/`wontfix` (never `ported`,
never `ignore`). See `README.md`'s "Tracking: plan.sqlite3" section for the full column/status
reference.

The same no-chat-memory rule applies here: a finding from an audit (your own, a subagent's, or a
prior session's) is only real once it's written into `ticket.notes` or a commit — not while it
exists only as something said in this conversation. Write detailed `notes` on every ticket you
touch (what was checked, what was found, commit hash, what was deliberately deferred and why) so a
future session with zero memory of this one can trust the ticket's status without re-investigating
it from scratch. See README.md's "Ticket completion checklist" for exactly what "done" requires.

Process one ticket at a time, in priority order (`P0` → `P1` → `P2` → `P3`, then `ticket_no`):

```sql
SELECT ticket_no, priority, category, area, title FROM ticket
WHERE status='todo' ORDER BY priority, ticket_no LIMIT 1;

UPDATE ticket SET status='doing', updated_at=datetime('now') WHERE ticket_no=<N>;
-- ... do the work, run the ticket's validation_command or the standard build+test ...
UPDATE ticket SET status='done', updated_at=datetime('now'),
  notes=COALESCE(notes,'') || '\nDone: <short summary>' WHERE ticket_no=<N>;

-- Can't proceed for an external/technical reason:
UPDATE ticket SET status='blocked', updated_at=datetime('now'),
  notes=COALESCE(notes,'') || '\nBlocked: <exact reason>' WHERE ticket_no=<N>;

-- Genuinely ambiguous architecture decision — do not guess:
UPDATE ticket SET status='needs_user', updated_at=datetime('now'),
  notes=COALESCE(notes,'') || '\nNeeds user: <exact question>' WHERE ticket_no=<N>;

-- Progress summary:
SELECT status, priority, COUNT(*) FROM ticket GROUP BY status, priority ORDER BY priority, status;
```

Same autonomy rule as the `task` workflow above: don't stop between tickets to ask permission. Use
`blocked`/`needs_user` (with a precise reason in `notes`) instead of guessing on a genuine
architecture question, and keep going to the next ticket rather than getting stuck. Commit
ticket-driven code/doc changes in focused, ticket-scoped commits (batching only when several
tickets in the same run touch the same file for the same reason).
