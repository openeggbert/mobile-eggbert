# Daily stabilization summary template

Copy this template into a new section at the **top** of `NEXT.md` (above the previous
checkpoint) at the end of each stabilization session, whether the session is interactive
or autonomous. This formalizes the structure `NEXT.md`'s checkpoint sections have already
been following — see any existing "## Session checkpoint (...)" section in `NEXT.md` for
a filled-in example.

Keep it factual and specific (commit hashes, ticket numbers, exact counts) rather than
vague ("various fixes") — a future session (possibly with no memory of this one) needs to
be able to resume from this alone, without re-reading the full session transcript.

```markdown
## Session checkpoint (YYYY-MM-DD) — <one-line theme, e.g. "ticket queue progress">

*Branch: `feature/work`, HEAD `<short-hash>` — <N> tests passing, full clean rebuild
verified (0 errors/0 warnings)*

### Ticket queue progress
<Before → after counts per priority, e.g. "P1 todo: 63 → 19">

### What was fixed (real bugs, not just documentation)
For each substantive fix, one bullet with: what was wrong, how it was verified against
the real .NET reference source (not assumed), the commit hash, and the ticket number.
Distinguish real behavioral fixes from doc-only/audit-only tickets explicitly — don't let
a "verified clean, no fix needed" ticket read the same as a "found and fixed a crash."

- **Ticket #<N> (<type/area>)**: <what was wrong> — verified against
  `<path in /rv/tmp/runtime/src/libraries/...>`. <What changed and why>. Commit `<hash>`.

### What was audited and found clean (no fix needed)
Brief list — these still count as real verification work, just don't pad the "fixed"
section with them.

### What was found but deliberately NOT fixed this session, and why
Real, confirmed gaps that were scoped out — e.g. because they're genuine new-feature work
disproportionate to a bug-fix pass, or because they need a user decision first. Say
exactly what's missing and where, so a future session doesn't have to re-discover it.

### Known open decisions (blocking further work in some area)
Anything that needs the user's input before more automated work can proceed safely —
architecture choices, scope boundaries, anything CLAUDE.md rule #10 ("no broad refactor
without discussion") would apply to. State the decision needed and the options, not just
"TBD."

### Process notes for future sessions
Anything non-obvious learned about *how* to do this work safely and well this session —
e.g. a subagent misbehaving, a stale audit finding that needed re-verification, a
build/test quirk. Skip this section if nothing new was learned.

**To resume cold, from a fresh context:** read `CLAUDE.md`, this file, and `prompt.md`,
then run:
\```bash
sqlite3 plan.sqlite3 "SELECT ticket_no, priority, category, title FROM ticket WHERE status='todo' ORDER BY priority, ticket_no LIMIT 10;"
\```
```

## Notes on filling it in

- **Commit hashes and ticket numbers are load-bearing.** A future session (or a future
  you, with no memory of this one) needs to `git show <hash>` or query
  `plan.sqlite3`/`ticket` for the exact detail — vague summaries force needless
  re-investigation.
- **Distinguish "fixed" from "audited clean" from "found but deferred."** All three are
  real, valuable outcomes, but conflating them makes it impossible to tell at a glance
  what still needs attention.
- **State exception-message/HResult/algorithm verification explicitly** ("verified
  against `/rv/tmp/runtime/src/libraries/.../Foo.cs`") rather than implying it — this is
  what lets a future session trust a "CLEAN" verdict instead of re-auditing from scratch.
- Don't restate CLAUDE.md's standing rules (zero-warnings, SPDX headers, etc.) — this
  file is for *what happened*, not the process definition. `CLAUDE.md`/`prompt.md` are
  the process definition.
