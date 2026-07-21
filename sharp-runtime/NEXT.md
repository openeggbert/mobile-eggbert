# NEXT.md

*Last updated: 2026-07-14. Branch: `feature/work`. HEAD is this checkpoint commit itself; the
last code-affecting commit was `8514a9d` (A-05 `OrderedDictionary` fix), followed by a docs-only
fix at `6aa719f` (A-06 README correction) — see §3.*

This document was rewritten from scratch on 2026-07-13 into a structured handoff format,
replacing a ~6000-line chronological session log. That log is not lost — it's in git history
(`git log --oneline -- NEXT.md`) and in `POST_STABILIZATION_AUDIT.md`. Condensed again on
2026-07-14 (the prior structured version had regrown to ~1200 lines of session narrative); this
file intentionally contains only the current, load-bearing state — not a blow-by-blow log.

---

## 1. Project summary

**sharp-runtime** is a C++23 static library that reimplements a practical subset of .NET's
`System.*` namespace, so ported C#/XNA game code compiles against C++ headers with minimal
changes. Foundation for **CNA** (a C++ XNA port) and **mobile-eggbert** — neither lives in this
repo; this is a standalone dependency.

**Main goal**: broad, behaviorally-faithful coverage of the .NET BCL surface a 2D/3D game
realistically needs, as idiomatic modern C++, not a literal transliteration.

**Current phase**: post-stabilization. The original porting/stabilization backlog
(`plan.sqlite3`'s `ticket` table) is fully complete. A subsequent fresh audit
(`POST_STABILIZATION_AUDIT.md`) found 20 issues; all fixed. No open, pre-scoped work item exists
anywhere in `plan.sqlite3` — the project is stable and awaiting direction for the next body of
work (§8 for candidates, §10 for a resume prompt).

**Key architectural decisions**:
- `SharpRuntime::intcs`/`longcs`/`shortcs`/`uintcs`/`ulongcs`/`ushortcs`/`bytecs`/`sbytecs`
  (fixed-width aliases in `include/SharpRuntime/SharpRuntimeHelper.hpp`) are mandatory in every
  public API that mirrors a .NET `int`/`long`/etc. Rollout is complete project-wide.
- Properties are `getXxxProperty()`/`setXxxProperty()`, with one documented exception: C#
  indexers map to `getItem()`/`setItem()`. `getCurrent()` → `getCurrentProperty()` was the last
  naming gap, closed this session across 23 files/61 occurrences — **a deliberate breaking
  change, no backward-compat alias**.
- A fixed set of .NET subsystems is permanently out of scope: reflection, GC internals, delegate
  `DynamicInvoke`, serialization infra, P/Invoke, symmetric/asymmetric crypto + TLS + X.509 (hash
  algorithms remain in scope). Full list in `CLAUDE.md`.
- A fixed set of subsystems are POSIX/Linux-only by necessity: `System::Net::Sockets`,
  `System::IO::RandomAccess`, `AppDomain`/`AppContext`, `TimeZoneInfo`, `Diagnostics::Process`,
  `PosixSignal`/`PosixSignalRegistration`, `NetworkInterface`, `FileSystemWatcher`. All correctly
  throw `PlatformNotSupportedException` elsewhere (verified by review, not by an actual
  Windows/Emscripten compile — see §5).
- Every mutable generic collection (`List`, `Dictionary`, `HashSet`, `LinkedList`, `Queue`,
  `Stack`, `SortedDictionary`, `SortedList`, `SortedSet`, `OrderedDictionary`) tracks a
  `version_` counter, checked by its enumerator on every step — mutating during enumeration
  throws `InvalidOperationException`, matching .NET's fail-fast contract. New invariant this
  session; see §6 for what it means for new collection types.

---

## 2. Current status

**Build**: clean — 0 errors, 0 warnings — at `6aa719f` (last commit, docs-only), via
`cmake --build build --parallel 4`.

**Tests**: **12476/12476 passing** at `8514a9d` (last code-touching commit), grown from a 12173
session-start baseline (~300 new regression tests this session).

**Sanitizers**: the full trio (TSan/ASan/UBSan) is verified clean project-wide. TSan has been
re-run repeatedly against each new concurrency-sensitive change, most recently the
`Task::ContinueWith`/`WhenAny` rewrite (`557d0ea`, 0 warnings); ASan was re-run against the same
change to verify its new `weak_ptr`-based cycle-avoidance design doesn't leak (0 leaks). UBSan
hasn't been re-run since the original pass at `1cdc80a` — low priority, no UB-prone patterns
introduced since. The `audit.md` remediation (A-01–A-06) has **not** had a dedicated sanitizer
re-run yet (candidate: A-02's destructor paths, A-05's exception-rollback logic — see §8).

**Targets**: single static library `SHARP_RUNTIME` + `SharpRuntimeTests` (GoogleTest). No CLI,
sample, or demo binary exists in this repo.

**Not yet done/verified**:
- No Windows or Emscripten build has ever actually been compiled (this sandbox is Linux-only).
  `#ifdef` branches for those platforms are code-reviewed only.
- Performance work: only `String` (`Split`/`Concat`/`Join`, optimized ~2.6x/~1.4x), `Dictionary`,
  `List<T>`, `StringBuilder` (all three measured, no significant win found) have been profiled.
  Everything else in the codebase is unmeasured.
- Known API gaps: `ImmutableList`'s missing LINQ-family methods; `TaskT<TResult>::ContinueWith`
  (see §5).

---

## 3. Recent changes

Scoped to this session's most significant work. A much larger backlog — 1709 tickets covering the
original porting/stabilization pass — was completed earlier in the same session and isn't
re-itemized; see `git log` for the full trail and exact per-commit test-count deltas.

**Foundational fixes and additions**:
- `POST_STABILIZATION_AUDIT.md` — fresh independent audit, 20 findings, all fixed.
- `version_` tracking + fail-fast enumeration added to 10 mutable collection types (see §1).
- `NumberStyles`-aware `Parse`/`TryParse` for all 8 integer types — `Integer`, `HexNumber`,
  `Number`, `Currency`, `BinaryNumber` now fully supported (`AllowExponent` is the one remaining
  gap, and real .NET's `Number`/`Currency` styles don't include it either — a non-gap).
- Correctness fixes: `Convert::ToXxx(double/float)` round-to-nearest (was truncate);
  `Span`/`ReadOnlySpan` indexer now throws `IndexOutOfRangeException`; `Dictionary`/
  `SortedDictionary`/`SortedList` non-const indexers now throw `KeyNotFoundException` instead of
  silently inserting; `ConcurrentDictionary::GetOrAdd`/`AddOrUpdate` reentrancy deadlock;
  `MemoryStream`/compression-stream write validation (a negative offset previously caused a
  confirmed OOB read) and disposed-state tracking; `Math::Round` no longer depends on ambient
  `fesetround()`; `StringInfo` UTF-8-aware text-element handling; `FileSystemWatcher` now throws
  `PlatformNotSupportedException` on non-Linux instead of silently no-opping.
- `getCurrent()` → `getCurrentProperty()` rename — 23 files, 61 occurrences, one commit, no
  compat alias (see §1).

**`Task`/async completeness** (`9d4e77e`, `2be9c3c`, `200591b`, `557d0ea`):
- Added `Task::WhenAll`, `Task::WhenAny`, `TaskCompletionSource<TResult>.Task`, and
  `Task::ContinueWith` (with `WhenAny` rebuilt on top of it, achieving zero extra OS threads —
  matches real .NET's `TaskFactory.CommonCWAnyLogic` completion-registration strategy instead of
  a dedicated watcher thread per input task).
- Real bugs found and fixed along the way: `WhenAny` originally `join()`'d losing watcher
  threads, making it as slow as the slowest input (fixed via `detach()`); a genuine
  `~TaskCompletionSource()` deadlock from GCC/libstdc++'s `shared_future` destruction semantics;
  an exception-type-sniffing bug (`catch (const TaskCanceledException&)` used to distinguish
  cancellation from a fault) that misclassified a task that legitimately *threw*
  `TaskCanceledException`, found independently in both `TaskCompletionSource::SetException`
  (`eb8489a`) and `Task::WhenAll` (`886ea61`); a `weak_ptr`-vs-reference-cycle leak risk in
  `ContinueWith`'s continuation-registration design (fixed before landing, verified leak-free via
  ASan); missing null/moved-from-`Task` validation in `WhenAny`/`WhenAll` (`498fa71`).
- `Channel<T>::CreateUnboundedPrioritized` added (`3a8adfa`); both channel types' `TryRead`/
  `TryWrite` had `notify_one()` lost-wakeup bugs under multiple blocked waiters, fixed to
  `notify_all()` (`41c0476`).

**Sanitizer trio — first-ever full runs in this project's history**:
- ThreadSanitizer: 1 real production deadlock (`Channel` missing `notify_all`) + 3 test-only
  races from `sleep_for`-as-synchronization — all fixed.
- AddressSanitizer: 1 confirmed heap-buffer-overflow (`NativeMemory::AlignedRealloc` copying the
  new size from an old, smaller allocation) + 5 leaked allocations in the XML subsystem
  (`XmlDocument::CreateAttribute`/`CreateEntityReference` had no owner) — all fixed.
- UndefinedBehaviorSanitizer: 1 UB call (empty-vector `.data()` reaching `fwrite`'s `nonnull`
  parameter inside vendored miniz) — fixed at this project's own call site.
- All three now run clean project-wide. A `-Wshadow` diagnostic build also came back clean
  (3 harmless test-file coincidences, zero in `include/`/`src/`) — no code changed.

**Performance audits** (measure-first discipline; `bench/StringBenchmark.cpp`, gated behind
`SHARP_RUNTIME_BUILD_BENCHMARKS`, default OFF — no vendored benchmarking library):
- `String::Split(char)`/`Concat`/`Join`: optimized (~2.6x / ~1.4x faster); also fixed a
  correctness bug found as a side effect (`Split("", ',')` now returns `{""}`, matching .NET,
  not an empty vector).
- `List<T>`, `StringBuilder`, `Dictionary<K,V>` (`Add`/indexer-setter): measured, no significant
  win found in any of the three — no code changed. Three honest negative results this session,
  not gaps (see CLAUDE.md's "no speculative optimization" rule).

**Duplicated-implementation audit round** — comparing near-identical code across sibling types
surfaced 5 real bugs, all fixed with regression tests: unsigned integer parser accepted repeated
sign tokens (`edb77f0`); `OrderedDictionary`'s non-const indexer inserted a default value on a
missing-key read, the same bug class already fixed on `Dictionary`/`SortedDictionary`/
`SortedList`/`ConcurrentDictionary` (`e48c381`); `OrderedDictionary::EnsureCapacity` didn't bump
`version_` (`31b77f6`); `UInt32` was the one integer type missing `ToString(value, format)`
(`8adb63e`); `Int16` was the one signed integer type missing `CopySign`/`IsNegative`/
`IsPositive`/`MaxMagnitude`/`MinMagnitude` (`68d1068`). Follow-up checks confirmed the
indexer-insert bug is now closed across `Collections::Specialized` too, and found no third
instance of the exception-type-sniffing pattern beyond the two already fixed.

**External audit remediation** (`audit.md`, 2026-07-14, untracked report in repo root) — 6 of 7
findings fixed:
- **A-01** (High, `221bb92`): stream-backed `ZipArchive` never wrote Create/Update output back to
  its `Stream*`; a second bug found while testing it — Update mode silently dropped all
  pre-existing entries. Both fixed.
- **A-02** (High, `f1266ae`): 5 destructors (`DeflateStream`/`GZipStream`/`ZLibStream`/
  `ZipArchive`/`XmlWriter`) could call `std::terminate()` on an ordinary I/O failure during RAII
  cleanup — wrapped in `try/catch(...)`.
- **A-03** (High, `5cf5fa5`): `DeflateStream`/`GZipStream`/`ZLibStream::Read()` had no buffer
  validation, unlike their own `Write()` — added the same checks.
- **A-04** (Medium, `95e88f0`): `TaskCompletionSource<TResult>::TrySetResult` could strand every
  waiter forever if `TResult`'s copy constructor threw mid-`set_value()` — fixed with a
  catch-and-fulfill-then-rethrow.
- **A-05** (Medium, `8514a9d`): `OrderedDictionary` could desync `entries_`/`keyIndex_` if a hash
  or key copy threw partway through a mutation — added transactional rollback.
- **A-06** (Low, `6aa719f`): README overclaimed Doxygen/tooling coverage — reworded, no code
  change.
- **A-07** (Low, deferred): no tracked CI pipeline — offered to the user (basic/comprehensive/
  skip), declined for now. See §5/§9.

---

## 4. Current blocker / main problem

**No active build/test blocker.** Build clean, 12476/12476 tests passing at the last
code-touching commit (`8514a9d`); re-verified after two docs-only commits at `6aa719f`.
`plan.sqlite3`'s `ticket` table has zero `blocked`/`todo`/`doing` rows; the `task` table has zero
unclassified rows.

This session has run autonomously (per explicit user authorization) through NEXT.md's original
§8 tasks plus several self-directed audit rounds — see §3. Two standing user decisions remain in
effect: (1) no new benchmarking dependency — `std::chrono`-only timing, per
`bench/StringBenchmark.cpp`; (2) push after each verified task.

**The open question is direction, not a technical problem** — what to work on next (see §8).

If you're resuming and something is failing, trust the failing command's own output over this
document, and update this section once you understand what changed.

---

## 5. Known bugs and limitations

**Deliberately deferred (documented scope decisions, not bugs)**:
- `ImmutableList<T>` is missing `Sort`/`Reverse`/`ForEach`/`CopyTo`/`GetRange`/`ConvertAll`/the
  `Find` family/`ToBuilder`/comparer overloads — cataloged in its class doc-comment.
- `Span<char>`/UTF-8 `ReadOnlySpan<byte>`-based `Parse`/`TryParse` overloads for the 8 integer
  types — not implemented, explicitly out of scope for the ticket that added the
  `NumberStyles`-aware overloads.
- `System::Xml::Linq::XText::WriteTo` doesn't distinguish `WriteWhitespace` vs `WriteString`
  under an `XDocument` parent the way real .NET does — needs a `WriteWhitespace` primitive
  `XmlWriter` doesn't have yet.
- `TaskT<TResult>::ContinueWith` (generic-result counterpart of `Task::ContinueWith`) is not
  implemented — `WhenAny`/`WhenAll` only operate on non-generic `Task` and didn't need it. The
  `completionMutex`/`completionCv` groundwork already exists on `TaskT::State`; missing is a
  `continuations` list + the method itself, mirroring `Task`'s `weak_ptr`-based cycle avoidance.
- **No tracked CI pipeline** (external audit finding A-07, 2026-07-14) — no
  `.github/workflows/*.yml` exists; `scripts/local_ci_check.sh` is the local-only equivalent.
  Offered to the user and declined for now. Don't add CI config unilaterally (see §9).
- `MemoryStream(const bytecs* buffer, intcs size)` defaults `writable_(false)`, but real .NET's
  single-array-arg constructor defaults to writable (`this(buffer, true)`). Found incidentally
  during A-01's regression tests, deliberately not fixed there to avoid scope creep. Well-scoped
  fix if picked up: change the default, audit callers/tests relying on the current behavior.

**Needs verification (unknown status)**:
- No Windows or Emscripten build has ever been compiled for this repo — every platform `#ifdef`
  branch is unverified beyond code review.
- Performance is measured only for `String` (optimized), `Dictionary`, `List<T>`, `StringBuilder`
  (measured, no win found) — see §3. Everything else is unmeasured.
- UndefinedBehaviorSanitizer hasn't had a dedicated re-run since `1cdc80a` (12378 tests) — low
  priority, no UB-prone patterns introduced since, and TSan/ASan (more relevant to the newest
  concurrency/memory-lifetime-heavy changes) are both freshly re-verified clean.

**Permanent (by design, not something to "fix")**:
- Reflection, GC internals, most delegates' `DynamicInvoke`, serialization infra, P/Invoke/
  interop, symmetric/asymmetric crypto + TLS + X.509 — all permanently out of scope. See
  `CLAUDE.md`.
- `System::Decimal`/`Int128`/`UInt128` require the GCC/Clang `__int128` extension and
  hard-`#error` on MSVC — permanent, accepted (2026-07-11 decision); not to be "fixed" with
  hand-rolled 128-bit arithmetic.
- `getCurrent()` → `getCurrentProperty()` has **no backward-compat alias** — any code (including
  CNA) calling the old name won't compile until updated on the consumer side. Explicit decision.
- `TypedReference` — re-verified 2026-07-13: correctly `status='ignore'`/`outofscope=1`. Its
  functionality depends entirely on C# compiler intrinsics and CLR reflection, squarely inside
  the "reflection is out of scope" deviation. False alarm from an earlier audit pass, not a bug.

---

## 6. Architecture notes

**Layout**:
- `include/System/...` — public headers, mirroring the .NET namespace hierarchy 1:1.
- `src/System/...` — `.cpp` bodies for complex types (simple types are header-only).
- `tests/System/...` — GoogleTest suites, generally mirroring `include/`.
- `vendor/` — third-party sources (GoogleTest, nlohmann/json, tinyxml2, miniz) — **exempt** from
  this project's SPDX-header, doc-comment, and naming rules.
- `plan.sqlite3` (gitignored) — two-table (`task`, `ticket`) work-tracking database. See
  `prompt.md` for the full workflow.

**Build**: CMake with `GLOB_RECURSE` auto-discovering `src/*.cpp` — a new `.cpp` file needs a
`cmake .` re-run in the build dir, no `CMakeLists.txt` edit.

**Invariants that must be preserved by any new code**:
1. Every public API parameter/return mirroring a .NET `int`/`long`/`short`/`byte`/etc. uses the
   matching `SharpRuntime::*cs` alias, never the raw C++ type. Enforced project-wide.
2. Every property getter/setter is `getXxxProperty()`/`setXxxProperty()`; the only exception is
   `getItem()`/`setItem()` for C# indexers. Zero other exceptions exist — don't introduce one
   without updating `CLAUDE.md`.
3. Any mutable collection-like type with an enumerator/iterator tracks a `version_` counter,
   bumped on structural mutation (not on value-only updates), checked every enumerator step,
   throwing `InvalidOperationException("Collection was modified; enumeration operation may not
   execute.")` on mismatch. Reference: `List<T>`, `Dictionary<K,V>`, `ArrayList`/`Hashtable`.
   **Note**: this port has twice bumped `version_` on `Remove()`/`Clear()` even where real .NET
   doesn't, because `std::unordered_map`/`set` iterators are invalidated by erase/clear in ways
   .NET's array-backed entries aren't — verify against both the .NET reference and actual C++
   iterator-invalidation rules when adding this to a new type, don't copy .NET blindly.
4. Any dictionary-like `operator[]` must distinguish get-intent (throw `KeyNotFoundException` on
   a missing key) from set-intent (insert on missing key) via a `ValueProxy` wrapper — see
   `ConcurrentDictionary.hpp`, `Dictionary.hpp`, `SortedDictionary.hpp`, or `SortedList.hpp`.

**Boundaries that must not be broken**:
- No POSIX `#include`s (`<unistd.h>`, `<sys/socket.h>`, etc.) in public `.hpp` headers — only in
  `.cpp` files behind `#ifdef _WIN32` / `#elif defined(__EMSCRIPTEN__)` / `#else`. Throw
  `PlatformNotSupportedException` on unsupported platforms, never silently degrade.
- No LINQ-style chaining in code this project writes internally — use `std::ranges` (auditing/
  porting an existing `System::Linq` surface is different and fine).
- SPDX header on every file under `include/`, `src/`, `tests/` (not `vendor/`).

**Compatibility/API rules**:
- Push only to `feature/work`. Never `develop`/`master`, never tags, without explicit per-action
  approval.
- `getCurrent()` → `getCurrentProperty()` is a completed, intentional breaking change — don't
  reintroduce the old name.
- No further broad, project-wide renames/refactors without the same explicit, per-action
  authorization the `intcs` and `getCurrentProperty()` changes required (CLAUDE.md rule #10).

---

## 7. Useful commands

```bash
# Configure + build (from repo root)
cmake -S . -B build
cmake --build build --parallel 4      # use --parallel 2 if the machine is thermally constrained

# Run the full test suite
./build/SharpRuntimeTests

# Run one suite/type's tests only
./build/SharpRuntimeTests --gtest_filter="TypeName*"

# Repeat a suite to check for flakiness (useful after any concurrency-related change)
./build/SharpRuntimeTests --gtest_filter="*Concurrent*" --gtest_repeat=5

# CI-parity check (clean build + zero warnings + full suite, mirrors CLAUDE.md rules #1/#2)
./scripts/local_ci_check.sh

# Errors/warnings only, from a build log
cmake --build build --parallel 4 2>&1 | grep -E "error:|warning:" | grep -v "^#"

# Reconfigure after adding a new .cpp file (GLOB_RECURSE needs a re-run to pick it up)
cd build && cmake . && cd ..

# Generate API docs (output already exists under docs/generated/ from a prior run)
doxygen Doxyfile
```

**Lint/format**: no `.clang-format` or other formatter/linter configuration exists in this repo.

**Reproduce the current bug**: not applicable — no known failing reproduction exists (see §4).

**Most important demo/sample**: none exists. `./build/SharpRuntimeTests` is the closest thing to
a "does this actually work" check available in this repo.

---

## 8. Next smallest tasks

Completed this session: all of NEXT.md's original tasks, a full post-pilot audit round, the
sanitizer trio (8 real bugs fixed, all three clean project-wide now), a `-Wshadow` audit (clean),
`Task::WhenAll`/`WhenAny`/`ContinueWith`/`TaskCompletionSource.Task`, several further audit rounds
(fresh-eyes self-review, duplicated-implementation search, external `audit.md` remediation), and
performance passes on `String`/`List<T>`/`StringBuilder`/`Dictionary<K,V>`. Full detail and commit
hashes are in §3.

None of the tasks below are currently blocking anything — pick based on what's actually wanted
next, or ask the user first if unsure which to prioritize.

1. **`TaskT<TResult>::ContinueWith`** (§5) — generic-result counterpart of `Task::ContinueWith`,
   deliberately deferred since `WhenAny`/`WhenAll` never needed it. Groundwork already exists on
   `TaskT::State`; a well-scoped completeness step, not an architectural unknown.

2. **`MemoryStream(buffer, size)` writability parity fix** (§5) — real .NET's single-array-arg
   constructor defaults to writable; this port defaults to read-only. Small, well-scoped: change
   the default, then audit existing callers/tests that may implicitly rely on the current
   (wrong) behavior.

3. **A-07 CI pipeline, if the user revisits it** (§5) — was explicitly offered and declined for
   now. Don't add `.github/workflows/*.yml` without asking again — it activates real GitHub
   Actions runs against the repo (see §9).

4. **A dedicated sanitizer pass over the `audit.md` remediation** (§2) — A-02's destructor fixes
   and A-05's exception-unwind rollback logic in `OrderedDictionary` are natural TSan/ASan
   candidates, even though the changes themselves are single-threaded; hasn't been done yet.

5. **Another audit round, different category.** Already covered this session: TODO/FIXME
   markers, weak tests, resource-management/RAII, `plan.sqlite3` drift, memory safety/UB (full
   sanitizer trio), variable shadowing, fresh-eyes self-review, duplicated-implementation search,
   and the external `audit.md` remediation. Not yet covered: `-Wconversion`/`-Wsign-conversion` —
   skipped so far, since this codebase's pervasive intentional `intcs`/`size_t` conversions would
   make a blind run overwhelmingly noisy; would need a smarter triage approach first.

---

## 9. Do not do yet

- **No restarting the `intcs`/`getXxxProperty()` naming rollouts** — both complete, project-wide.
  Don't re-scan for "one more" instance without a specific, concrete finding first.
- **No backward-compatibility shim for the `getCurrent()` → `getCurrentProperty()` rename** —
  explicit, deliberate decision; downstream consumers fix their own call sites.
- **No Windows/Emscripten CI setup** — still explicitly out of scope per `CLAUDE.md`.
- **No TLS/`SslStream`, asymmetric cryptography, or X.509 implementation** — permanent,
  documented deviation.
- **No reflection, GC-internals, serialization-infrastructure, or P/Invoke implementation
  attempts** — permanent, documented deviations.
- **No speculative performance optimization without measurement first** — the `String` pilot
  (§3) measured with a standalone script before changing anything; follow the same discipline,
  using `bench/StringBenchmark.cpp` as the template.
- **No further broad, many-file refactors or renames** without the same explicit, per-action
  authorization the `intcs` and `getCurrentProperty()` changes required.
- **No mass rewrite or reformatting in a single commit** — keep changes small, reviewable, and
  scoped to one ticket/task per commit.
- **No merge to `master`/`develop`, and no tags**, without explicit per-action user approval.
- **No touching `Decimal`/`Int128`/`UInt128`'s `__int128`-based MSVC limitation** — permanent,
  accepted (2026-07-11 decision); do not attempt a hand-rolled 128-bit arithmetic workaround.
- **No adding `.github/workflows/*.yml` (or any other CI config) without asking first** — audit
  finding A-07 (2026-07-14) was explicitly offered and declined for now. This is a real
  infrastructure decision (it activates GitHub Actions runs against the actual repo), not a code
  fix — different in kind from the rest of this list.

---

## 10. Resume prompt

```
Read NEXT.md first. It reflects the repository state as of the last code-touching commit,
8514a9d (an OrderedDictionary exception-safety fix, audit finding A-05), followed by a docs-only
commit 6aa719f (README correction, A-06) and this checkpoint commit -- 12476/12476 tests, 0
errors/0 warnings.

This session just finished a remediation pass over an external audit report (audit.md, still
untracked in the repo root -- an external report, not project source). 6 of 7 findings fixed --
see §3 for the full list and exact commit hashes. A-07 (no tracked CI pipeline) was explicitly
offered to the user and declined for now -- do NOT add .github/workflows/*.yml without asking
again, it activates real GitHub Actions runs against the repo (see §9).

Re-verify the baseline first regardless:
cmake --build build --parallel 4 && ./build/SharpRuntimeTests

Neither TSan nor ASan has been re-run specifically against this audit remediation yet (§2/§8) --
a reasonable candidate if picked up, given A-02 touches destructor exception paths and A-05
touches exception-unwind rollback logic, both single-threaded but in TSan/ASan's usual
wheelhouse.

There is no known active blocker -- the open question is which of §8's candidate next tasks (or
something else entirely) to work on. Pick ONE task, inspect only the files it needs. Do not
refactor unrelated code, do not touch files outside that task's scope, and do not restart the
completed intcs/getXxxProperty() rollouts.

Implement the task, run its verification (clean build, zero warnings; full suite passes;
new/changed behavior has a regression test). If a fix rolls back a mutation on an exception path,
verify the test actually exercises that path by temporarily reverting the fix and confirming it
fails -- this session found two cases where a plausible-looking test passed for the wrong reason
(an unrelated std::vector reallocation masking the real bug).

Update NEXT.md after finishing: move the completed task out of §8, note what changed in §3, and
update the "Last updated" line and test count at the top of the file.
```
