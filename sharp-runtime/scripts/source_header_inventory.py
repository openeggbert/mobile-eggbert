#!/usr/bin/env python3
"""Inventories every include/**/*.hpp and src/**/*.cpp file: SPDX header
presence, line count, and (best-effort) the namespace/type name it implies,
cross-referenced against plan.sqlite3's task table so it's easy to spot
headers with no matching task row or task rows marked 'ported' with no
corresponding header file. Prints a summary and writes full detail to CSV.

Usage: scripts/source_header_inventory.py [--csv PATH] [--db PATH]
"""

import argparse
import csv
import os
import re
import sqlite3
import sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_DB = os.path.join(REPO_ROOT, "plan.sqlite3")
DEFAULT_CSV = os.path.join(REPO_ROOT, "source_header_inventory.csv")

SPDX_RE = re.compile(r"SPDX-License-Identifier:\s*MIT")
NAMESPACE_RE = re.compile(r"namespace\s+((?:System|SharpRuntime)(?:::\w+)*)\s*\{")
TYPE_RE = re.compile(
    r"^\s*(?:template\s*<[^>]*>\s*)?"
    r"(?:class|struct|enum(?:\s+class)?)\s+(\w+)",
    re.MULTILINE,
)


def collect_files():
    for base in ("include", "src"):
        base_path = os.path.join(REPO_ROOT, base)
        for root, _, files in os.walk(base_path):
            if "vendor" in root.split(os.sep):
                continue
            for f in files:
                if f.endswith((".hpp", ".cpp")):
                    yield os.path.join(root, f)


def inspect_file(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            content = fh.read()
    except OSError:
        return None

    lines = content.count("\n") + 1
    has_spdx = bool(SPDX_RE.search(content[:400]))
    ns_match = NAMESPACE_RE.search(content)
    namespace = ns_match.group(1) if ns_match else ""
    type_names = TYPE_RE.findall(content)

    return {
        "path": os.path.relpath(path, REPO_ROOT),
        "lines": lines,
        "has_spdx": has_spdx,
        "namespace": namespace,
        "types": ";".join(type_names[:5]),
    }


def load_ported_task_names(db_path):
    if not os.path.exists(db_path):
        return set()
    conn = sqlite3.connect(db_path)
    try:
        rows = conn.execute(
            "SELECT namespace, name FROM task WHERE status='ported'"
        ).fetchall()
    finally:
        conn.close()
    return {f"{ns}.{name}" for ns, name in rows}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", default=DEFAULT_CSV, help="Output CSV path")
    parser.add_argument("--db", default=DEFAULT_DB, help="plan.sqlite3 path")
    args = parser.parse_args()

    rows = []
    missing_spdx = []
    for path in sorted(collect_files()):
        info = inspect_file(path)
        if info is None:
            continue
        rows.append(info)
        if not info["has_spdx"]:
            missing_spdx.append(info["path"])

    ported_names = load_ported_task_names(args.db)

    with open(args.csv, "w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(
            fh, fieldnames=["path", "lines", "has_spdx", "namespace", "types"]
        )
        writer.writeheader()
        writer.writerows(rows)

    total_lines = sum(r["lines"] for r in rows)
    hpp_count = sum(1 for r in rows if r["path"].endswith(".hpp"))
    cpp_count = sum(1 for r in rows if r["path"].endswith(".cpp"))

    print(f"Files scanned:     {len(rows)} ({hpp_count} .hpp, {cpp_count} .cpp)")
    print(f"Total lines:       {total_lines}")
    print(f"Missing SPDX:      {len(missing_spdx)}")
    for p in missing_spdx:
        print(f"  - {p}")
    print(f"Tasks marked ported (plan.sqlite3): {len(ported_names)}")
    print(f"CSV written to:    {os.path.abspath(args.csv)}")

    return 1 if missing_spdx else 0


if __name__ == "__main__":
    sys.exit(main())
