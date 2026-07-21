#!/usr/bin/env python3
"""Index all public types from dotnet/runtime .cs files into a SQLite database."""

import os
import re
import sqlite3
import sys

SEARCH_DIRS = [
    "/rv/tmp/runtime/src/libraries",
    "/rv/tmp/runtime/src/coreclr/System.Private.CoreLib",
]

DB_PATH = "/rv/data/development/github.com/openeggbert/sharp-runtime/dotnet_types.db"

NS_RE = re.compile(r'^\s*namespace\s+([\w.]+)', re.MULTILINE)
TYPE_RE = re.compile(
    r'^\s*(?:public\s+)'
    r'(?:(?:static|sealed|abstract|partial|readonly|ref|unsafe|new|override|virtual|extern)\s+)*'
    r'(class|struct|enum|interface|delegate|record)\s+(\w+)',
    re.MULTILINE
)

def collect_files():
    for d in SEARCH_DIRS:
        for root, _, files in os.walk(d):
            for f in files:
                if f.endswith(".cs"):
                    yield os.path.join(root, f)

def parse_file(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as fh:
            content = fh.read()
    except OSError:
        return []

    namespaces = NS_RE.findall(content)
    types = TYPE_RE.findall(content)  # list of (kind, name)

    if not types:
        return []

    rel_path = path.replace("/rv/tmp/runtime/", "")
    primary_ns = namespaces[0] if namespaces else ""

    rows = []
    for kind, name in types:
        rows.append((rel_path, primary_ns, name, kind))
    return rows

def main():
    if os.path.exists(DB_PATH):
        os.remove(DB_PATH)

    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()
    cur.execute("""
        CREATE TABLE types (
            id        INTEGER PRIMARY KEY,
            file      TEXT NOT NULL,
            namespace TEXT NOT NULL,
            type_name TEXT NOT NULL,
            type_kind TEXT NOT NULL
        )
    """)
    cur.execute("CREATE INDEX idx_ns ON types(namespace)")
    cur.execute("CREATE INDEX idx_name ON types(type_name)")

    files = list(collect_files())
    total = len(files)
    inserted = 0

    for i, path in enumerate(files):
        if i % 2000 == 0:
            print(f"  {i}/{total} files processed...", flush=True)
        rows = parse_file(path)
        if rows:
            cur.executemany(
                "INSERT INTO types(file, namespace, type_name, type_kind) VALUES (?,?,?,?)",
                rows
            )
            inserted += len(rows)

    con.commit()
    con.close()
    print(f"Done. {total} files scanned, {inserted} type entries inserted.")
    print(f"DB: {DB_PATH}")

if __name__ == "__main__":
    main()
