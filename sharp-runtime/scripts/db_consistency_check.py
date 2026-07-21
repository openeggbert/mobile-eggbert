#!/usr/bin/env python3
"""Checks plan.sqlite3 for internal-consistency problems in the `task` and
`ticket` tables: invalid status values, status/outofscope mismatches,
duplicate/blank identifying fields, and ticket rows referencing an unknown
status. Prints each problem found and exits non-zero if any were found.

Usage: scripts/db_consistency_check.py [--db PATH]
"""

import argparse
import os
import sqlite3
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_DB = os.path.join(REPO_ROOT, "plan.sqlite3")

# Per CLAUDE.md's plan.sqlite3 workflow section. Both 'ignore' and 'ignored' are accepted:
# CLAUDE.md documents 'ignore', but 'ignored' is the actual dominant historical convention
# in this database (15052 rows vs. 137) — flagging that split as 15k "problems" would make
# this checker useless in practice, so both spellings are treated as valid synonyms here.
VALID_TASK_STATUSES = {"", "todo", "ported", "ignore", "ignored", "tobedecided"}
VALID_TICKET_STATUSES = {"todo", "doing", "done", "blocked", "needs_user", "wontfix"}
VALID_TICKET_PRIORITIES = {"P0", "P1", "P2", "P3"}


def check(conn):
    problems = []
    cur = conn.cursor()

    # --- task table ---
    cur.execute("SELECT id, namespace, name, type, status, outofscope FROM task")
    task_rows = cur.fetchall()
    for tid, ns, name, ttype, status, outofscope in task_rows:
        if status not in VALID_TASK_STATUSES:
            problems.append(f"task id={tid} ({ns}.{name}): invalid status {status!r}")
        if not name or not name.strip():
            problems.append(f"task id={tid}: blank name")
        if not ttype or not ttype.strip():
            problems.append(f"task id={tid} ({ns}.{name}): blank type")
        if outofscope not in (0, 1):
            problems.append(f"task id={tid} ({ns}.{name}): outofscope not 0/1 ({outofscope!r})")
        if outofscope == 1 and status not in ("ignore", "ignored"):
            problems.append(
                f"task id={tid} ({ns}.{name}): outofscope=1 but status={status!r} (expected ignore/ignored)"
            )

    dupes = cur.execute(
        """
        SELECT namespace, name, type, COUNT(*) c
        FROM task GROUP BY namespace, name, type HAVING c > 1
        """
    ).fetchall()
    for ns, name, ttype, c in dupes:
        problems.append(f"task duplicate: ({ns}, {name}, {ttype}) appears {c} times")

    # --- ticket table ---
    cols = [r[1] for r in cur.execute("PRAGMA table_info(ticket)")]
    if "ticket_no" not in cols:
        problems.append("ticket table missing expected column 'ticket_no'")
    else:
        cur.execute("SELECT ticket_no, title, status, priority FROM ticket")
        ticket_rows = cur.fetchall()
        seen_nos = {}
        for tno, title, status, priority in ticket_rows:
            if status not in VALID_TICKET_STATUSES:
                problems.append(f"ticket #{tno} ({title!r}): invalid status {status!r}")
            if priority not in VALID_TICKET_PRIORITIES:
                problems.append(f"ticket #{tno} ({title!r}): invalid priority {priority!r}")
            if not title or not title.strip():
                problems.append(f"ticket #{tno}: blank title")
            if tno in seen_nos:
                problems.append(f"ticket_no {tno} duplicated (titles: {seen_nos[tno]!r}, {title!r})")
            seen_nos[tno] = title

    return problems


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--db", default=DEFAULT_DB, help="plan.sqlite3 path")
    args = parser.parse_args()

    if not os.path.exists(args.db):
        print(f"FAIL: database not found at {args.db}", file=sys.stderr)
        return 1

    conn = sqlite3.connect(args.db)
    try:
        problems = check(conn)
    finally:
        conn.close()

    if not problems:
        print("OK: no consistency problems found")
        return 0

    print(f"FAIL: {len(problems)} consistency problem(s) found:")
    for p in problems:
        print(f"  - {p}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
