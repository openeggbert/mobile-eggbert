# Sharp Runtime

**Sharp Runtime** is a C++ reimplementation of a small C#/.NET runtime subset (mainly used by CNA and game ports).

The goal of this project is to provide a lightweight, .NET-inspired foundation layer for C++ projects, with a focus on:

* familiar API design (`System::*`-like namespaces)
* clean and modern C++ implementation
* compatibility with higher-level frameworks (e.g. CNA)

> ⚠️ This is **not** a full .NET runtime or CLR implementation.
> It is a pragmatic subset designed for use in native C++ applications.

---

# License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

## Attribution

Sharp Runtime is **partly derived from the .NET runtime**
([dotnet/runtime](https://github.com/dotnet/runtime), MIT License, © .NET Foundation and Contributors).

Specifically:
- The **public API design** of `System::*` types — class names, method signatures, namespace structure, and enum values — is based on the .NET standard library.
- Some **algorithmic implementations** (number formatting, date/time arithmetic, Unicode handling, etc.) are informed by or translated from the dotnet/runtime source code.
- The **C++ implementation** (headers, `.cpp` bodies, CMake build, tests) is original work by Robert Vokac and contributors.

The dotnet/runtime source is available at https://github.com/dotnet/runtime under the MIT License.

## Third-party components

This project vendors a small number of third-party libraries under `vendor/`, each under its
own permissive license (embedded in the vendored source itself, plus a standalone `LICENSE`
file where the upstream project provides one):

- [GoogleTest](https://github.com/google/googletest) (`vendor/googletest`) — BSD 3-Clause, `LICENSE` included.
- [nlohmann/json](https://github.com/nlohmann/json) (`vendor/nlohmann`) — MIT License, embedded in `json.hpp`.
- [tinyxml2](https://github.com/leethomason/tinyxml2) (`vendor/tinyxml2`) — zlib License, embedded in `tinyxml2.h`.
- [miniz](https://github.com/richgel999/miniz) (`vendor/miniz`) — MIT License, embedded in `miniz.c`.

None of these licenses are modified or removed from the vendored source.

## Permanent deviations from .NET

Some .NET features are intentionally out of scope and will never be ported — reflection
(`System.Type`, `Activator`, `Enum.GetNames`/`GetValues`), the GC, delegates beyond
`std::function`, serialization infrastructure, P/Invoke, and full symmetric/asymmetric
cryptography, X.509 certificates, and TLS (`SslStream`). See **[CLAUDE.md](CLAUDE.md)**
("Parity philosophy" section) for the complete list and the reasoning behind each one.

---

# 🚀 Goals

* Recreate useful parts of `.NET` API in idiomatic C++
* Provide building blocks such as:

    * exceptions
    * events / delegates
    * basic system types
* Serve as a foundation for higher-level frameworks (e.g. CNA)
* Keep the codebase simple, readable, and well-documented

---

# 🛠️ Build

```bash
git submodule update --init --recursive
cmake -S . -B build -DSHARP_RUNTIME_BUILD_TESTS=ON
cmake --build build --parallel 4
./build/SharpRuntimeTests
```

A library-only build (no test binary) is also supported:

```bash
cmake -S . -B build-no-tests -DSHARP_RUNTIME_BUILD_TESTS=OFF
cmake --build build-no-tests --parallel 4
```

## Build troubleshooting

- **`vendor/googletest is missing` (`FATAL_ERROR` from CMake)** — the git submodule wasn't
  initialized. Run `git submodule update --init --recursive` from the repo root.
- **A new `.cpp`/`.hpp` file doesn't seem to be picked up** — `src/*.cpp` and `tests/*.cpp` are
  auto-discovered via `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`, so a plain `cmake --build`
  re-runs CMake's configure step automatically when the file list changes (you'll see
  `-- GLOB mismatch!` in the build output). If a build still doesn't pick up a new file, force a
  reconfigure: `cmake -S . -B build` before building.
- **Isolating warnings from a large build log**:
  ```bash
  cmake --build build --parallel 4 2>&1 | grep -E "error:|warning:" | grep -v "^#"
  ```
  The build must produce zero output from this command before any commit (CLAUDE.md rule #1).
- **Running one test suite instead of the full ~10 900+ suite**: `./build/SharpRuntimeTests
  --gtest_filter="SuiteName.*"` (standard GoogleTest filter syntax — see
  `scripts/local_ci_check.sh` for a full build+test gate you can run locally before pushing).
- **Cross-compiling for Windows (MinGW) or Emscripten** — neither is part of the default build
  and neither has been wired into CI, but both are real, working, verified targets (not
  aspirational): `x86_64-w64-mingw32-g++` and `emcmake cmake` both build the `SHARP_RUNTIME`
  library target cleanly as of the fixes tracked under stabilization tickets #40 (Windows) and
  #41 (Emscripten) — see those tickets' notes in `plan.sqlite3` for the exact blockers found and
  fixed, and for what wasn't verified (the test binary itself is not built under either
  cross-compilation target — GoogleTest isn't cross-compiled for wasm/MinGW in this repo).
- **A header fails to compile when included on its own but works in the full build** — this
  usually means the header is relying on a transitive `#include` pulled in by whatever included
  it first, rather than including everything it uses itself. Check with:
  ```bash
  echo '#include "System/Some/Header.hpp"' > /tmp/tc.cpp
  g++ -fsyntax-only -std=c++23 -Iinclude -Ivendor /tmp/tc.cpp
  ```
  (`scripts/source_header_inventory.py` is a related but different check: it inventories
  SPDX/namespace/type metadata across every header and cross-references it against
  `plan.sqlite3`, useful for spotting headers with no matching task row — it does not invoke a
  compiler and cannot detect this class of transitive-include failure itself; corrected 2026-07-14,
  external audit finding A-06, after the script's own docstring was checked against this claim).
- **Generating API documentation**: `Doxyfile` at the repo root is configured to scan `include/`
  and `README.md` recursively (excluding `include/vendor`) and write HTML output to
  `docs/generated/html` (git-ignored — it's build output, not checked in):
  ```bash
  mkdir -p docs/generated && doxygen Doxyfile
  ```
  Open `docs/generated/html/index.html` in a browser. As of 2026-07-14 a run currently emits
  around 1,869 warnings (mostly undocumented members/parameters on older, pre-doc-comment-era
  code) — Doxygen coverage is an incremental, in-progress standard applied to newly-ported and
  newly-touched code (see CLAUDE.md's porting checklist), not yet a zero-warnings gate the way
  the compiler build is (CLAUDE.md rule #1). Treat a NEW warning introduced by your own change as
  a real issue to fix; the pre-existing backlog is tracked separately, not something any single
  change is expected to clear.

---

# 🗂️ Tracking: `plan.sqlite3`

Porting progress and stabilization work are tracked in a local, git-ignored SQLite database,
`plan.sqlite3`, with **two separate tables that must not be confused with each other**:

## `task` — .NET type classification

One row per type from [dotnet/runtime](https://github.com/dotnet/runtime)'s public surface. Tracks
*whether and how* a given `System.*` type has been dealt with.

| `task.status` | Meaning |
|---|---|
| `''` / `todo` | Not yet classified or ported. |
| `ported` | Implemented, tested, meets the full porting checklist. |
| `ignore` / `ignored` | Permanently out of scope (both values exist — `ignored` predates the current workflow and is left as-is, not "fixed" to `ignore`). |
| `tobedecided` | Genuinely ambiguous; needs a human architecture decision, not a guess. |

`in_progress` is **not** a valid `task.status` value.

## `ticket` — stabilization work queue

One row per concrete stabilization task (documentation fixes, correctness audits, platform checks,
test coverage, etc.) that isn't itself "port a .NET type." Independent of `task`.

| `ticket.status` | Meaning |
|---|---|
| `todo` | Not started. |
| `doing` | Actively being worked. |
| `done` | Complete — acceptance criteria met, build clean, tests passing/updated. |
| `blocked` | Can't proceed for an external/technical reason (recorded in `notes`). |
| `needs_user` | Requires a human decision that can't be made safely alone (recorded in `notes`). |
| `wontfix` | Deliberately not doing this (recorded in `notes`, e.g. permanent out-of-scope). |

`ticket.status` and `task.status` are **different systems with different value sets** — a ticket is
never `ported`, and a task is never `doing`.

```bash
# Next ticket to work on
sqlite3 plan.sqlite3 "SELECT ticket_no, priority, title FROM ticket WHERE status='todo' ORDER BY priority, ticket_no LIMIT 1;"

# Overall progress
sqlite3 plan.sqlite3 "SELECT status, priority, COUNT(*) FROM ticket GROUP BY status, priority ORDER BY priority, status;"
```

### Ticket completion checklist

A ticket may be marked `done` only when **all** of the following hold (this is the pattern
every stabilization ticket in this repo's history has actually followed — not aspirational):

1. **Claim verified against the real .NET source, not memory or an audit finding taken on
   faith.** If the ticket concerns behavior that should match .NET, the relevant file(s) under
   `/rv/tmp/runtime/src/libraries/` were actually read for this ticket — an audit agent's or a
   prior session's finding is a lead to verify, not a fact to act on directly (audit findings in
   this repo's history have been stale/wrong more than once; always re-check against current
   source before fixing).
2. **Real bugs fixed, not papered over.** If a genuine behavioral gap was found, either fix it or
   explicitly document *why not* (disproportionate scope, needs a user decision, permanent
   deviation) — never silently narrow the ticket's scope to avoid the harder part.
3. **Clean build.** `cmake --build build --parallel 4` — zero errors, zero warnings
   (`scripts/local_ci_check.sh` automates this check).
4. **Tests updated and passing.** Any test that encoded the *old* (buggy) behavior as expected is
   fixed, not left red or deleted. New regression tests exist for anything that was actually
   fixed — a test that would have caught the bug before the fix, not just a test that happens to
   pass after it. `./build/SharpRuntimeTests` — zero failures.
5. **Committed and pushed to `feature/work`** (never `develop`/`master` without explicit
   per-action approval) with a commit message that states what was wrong, how it was verified,
   and the ticket number.
6. **`plan.sqlite3` updated**: `status='done'`, `updated_at=datetime('now')`, and `notes` filled
   in with enough detail (what was checked, what was found, commit hash, what — if anything —
   was deliberately deferred and why) that a future session doesn't have to redo the
   investigation to know whether the ticket is *actually* resolved.

If any of 1–4 can't be satisfied safely (e.g. the fix needs a user decision per CLAUDE.md rule
#10, or the "bug" turns out to be a documented permanent deviation), the ticket should be
`blocked`, `needs_user`, or `wontfix` instead of `done` — never `done` with an unresolved caveat
buried in the notes.

### More `plan.sqlite3` queries

```bash
# Ticket counts broken down by status, priority, category, and area
sqlite3 plan.sqlite3 "SELECT status, priority, category, area, COUNT(*) FROM ticket
  GROUP BY status, priority, category, area ORDER BY priority, status, category;"

# Blocked / needs-user tickets, with the exact reason and file each is about
sqlite3 plan.sqlite3 "SELECT ticket_no, status, source_path, notes FROM ticket
  WHERE status IN ('blocked', 'needs_user') ORDER BY priority, ticket_no;"

# Completed tickets missing a validation_command or an updated_at timestamp
# (a hygiene check — every ticket marked done should have both; see the
# Ticket completion checklist above)
sqlite3 plan.sqlite3 "SELECT ticket_no, title FROM ticket
  WHERE status = 'done' AND (validation_command IS NULL OR validation_command = ''
  OR updated_at IS NULL) ORDER BY ticket_no;"
```

`plan.sqlite3` itself is a plain SQLite3 database file (rollback journal mode, no WAL
sidecar files, no virtual tables/extensions) — copying/backing it up is always safe as a
single-file copy, and it's readable with any standard SQLite client.

---

# 📊 Implementation Status Convention

Doxygen `@note Status: ...` comments on individual classes/methods are a **secondary, human-readable
hint**, not the source of truth — `plan.sqlite3`'s `task` table is authoritative for whether a type
counts as ported. These statuses are not enforced by the compiler.

## Status values

* **Todo** — not implemented yet
* **Stub** — skeleton only, returns placeholder or fails
* **Partial** — partially implemented; compiles and mostly works but has known, documented gaps
* **Implemented** — functionally complete
* **Verified** — validated against expected .NET behavior

---

# 📝 Comment Format

Each class or function may include a status note:

```cpp
/**
 * @note Status: Partial
 */
```

Full example:

```cpp
/**
 * @brief Provides 2D sprite rendering functionality.
 *
 * @note Status: Partial
 */
class SpriteBatch
{
public:
    /**
     * @brief Begins a sprite drawing batch.
     *
     * @note Status: Implemented
     */
    void Begin();

    /**
     * @brief Draws a texture at the specified position.
     *
     * @note Status: Partial
     */
    void Draw(Texture2D& texture, Vector2 position, Color color);

    /**
     * @brief Ends a sprite drawing batch.
     *
     * @note Status: Todo
     */
    void End();
};
```

---

# 🧠 Design Philosophy

* Prefer clarity over completeness
* Avoid unnecessary complexity
* Keep APIs close to .NET where it makes sense
* Use modern C++ (RAII, strong typing, clear ownership)

---

# ⚠️ Scope

Sharp Runtime intentionally **does not aim to implement:**

* the CLR (Common Language Runtime)
* JIT compilation
* full .NET standard compatibility

Instead, it focuses on a **practical subset** useful for native development.

---

# 🔗 Related Projects

* CNA — C++ reimplementation of XNA 4.0 (built on top of this library)

