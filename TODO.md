# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up (2026-05-12)

**First work:** Tony reviews `Tokf/Tests/FormatC.twk` via fileMerge against `Tokf/FormatC.twk` (overnight task). If the review passes, build and test Phase Triage: run tok on a sample .twk file using the Tests/-sandboxed FormatC, verify the resulting .h has include guards, `#include "gc/gc_cpp.h"`, and `: public gc` inheritance. If the build works and output looks right, promote Tests/FormatC.twk to live Tokf/FormatC.twk and rebuild ~/bin/tok.

**Tony's overnight work also includes:**
- Thinking about the code-block-as-defining-region problem (the `{...}` issue we backed out of checkSkip — defineSkipSet still declared, just not used yet).
- Reviewing the FormatC.twk Triage changes via fileMerge.

**Reading targets for Clay and Clod:**
- `Tokf/Tests/FormatC.twk` — the staged changes are here. Reading just the three edit sites is enough:
  - Line 651-652 area (GC inheritance in C++ class declaration)
  - Lines 174-190 (guardTokenFromName helper)
  - Lines 195-211 (close() with guard emission)
- `Tokf/FormatC.twk` — original for diff reference.
- `Groups/GroupRules.twk` — checkSkip() is post-fix; line 159 changed, comment-in-define-block now works. The defineSkipSet machinery is staged but not active.

**Out of scope (do not browse, survey, or modify):**
- `Groups/GUI/` — HPDL (Tony may unleash Clod-recon as a deliberate exception later; not today)
- `Parse/BeforeRefactor/` — archive

**Known current state:**
- One uncommitted local change to commit at session-start: the checkSkip comment fix (line 159: `atContent += 2` → `atContent++`; line 163: `atRuleMark = atContent + 1`).
- Tokf/Tests/FormatC.twk modified (Phase Triage staging) — stays dirty pending review.
- defineSkipSet declared in GroupRules.twk and initialized in GroupMain.twk but not yet used in checkSkip — Tony's overnight design work.
- Build still clean (Phase Triage hasn't been built/tested yet; comment fix is pure source-level).

**Cha cha step worth noting:**
This session introduced **Clay-designs, Clod-applies, Tony-reviews-before-tok-time** as a new mode. Used for Phase Triage today: edits land in a Tests/-sandboxed copy of FormatC.twk, Tony reviews via fileMerge before any tok run touches the modified file. Different from Tony-driving (yesterday's bisects) and Clay-and-Tony-debug-together (this morning's checkSkip walk-through). The discipline: any change that will pass through a code generator (tok or similar) gets staged for human review *before* the generator runs on it. Works well when there's enough overnight time for the review.

---

## 🔥 Immediate (current sprint)

### Phase Triage — staged, pending build verification

*Tokf/Tests/FormatC.twk has three edits: GC inheritance in C++ class declarations (lines 623-628 area), guardTokenFromName helper function (new), and close() modified to emit include-guard prologue + `#endif`. Live Tokf/FormatC.twk is byte-identical to its pre-session state (MD5 6e473417dcbf31c5107cef0f80cbe82e). Tomorrow's first work is fileMerge review + build/test + promotion.*

- [x] Recon: identify edit sites in FormatC.twk (Tonto, 2026-05-11)
- [x] Design: three edits — GC inheritance, helper, close() modification
- [x] Apply edits to Tokf/Tests/FormatC.twk (sandbox)
- [ ] Tony reviews via fileMerge
- [ ] Build ~/bin/tok from Tests/FormatC.twk (sandbox build)
- [ ] Run sandboxed ~/bin/tok on a sample .twk file (one of the support classes?)
- [ ] Verify output: include guards in .h, `#include "gc/gc_cpp.h"`, `: public gc` inheritance for C++ classes
- [ ] If clean, promote Tests/FormatC.twk to live Tokf/FormatC.twk, rebuild ~/bin/tok
- [ ] Test against existing test corpus to confirm no regressions

### checkSkip — code-block-as-defining-region problem (Tony's overnight)

*Half of indent-as-structure handling: comments now work (fixed today). Code blocks (`{...}`) inside define mode still problematic: the synthetic-`:` machinery clobbers the first character inside `{`. defineSkipSet was sketched as a fix but caused regressions in `setBrackets` (the `{` in `[{A-Za-z0-9+-;(]` set declarations was treated as a brace block). The actual fix needs context-awareness — `{` should be treated as a brace-block only in defining-mode contexts, not inside `[...]` set declarations or other contexts where `{` is content.*

- [ ] Tony's overnight: design a context-aware approach to `{` handling in checkSkip
- [ ] Apply, test against the drawing file's Point + arc + curve definitions
- [ ] Verify no regression in setBrackets or anywhere else `{` appears as content

### Xcode environment — workable dev loop ✅ DONE (2026-05-11)

*Tony got the build working overnight. Goal: workable dev loop in Xcode. Achieved.*

### Incant — PLGitem migration vet ✅ DONE in effect (2026-05-11)

*Build came clean when Tony got the dev loop working — meaning the PLGitem migration items in GroupItem.mm were either already addressed in his overnight work or the issues resolved as side effects of other changes. The catalogued errors (compare×3, amount, test) no longer appear in the build output.*

### TAWK — Back Up and Running

*Get TAWK incorporating the revised PLG. Floor POP: new TAWK processes the .twk files in plg/incant/support/tawk. Stretch POP: `plg Tawk.g` → Tawk.twk → new TAWK builds → new TAWK re-tokkifies the ecosystem. Self-consistent loop.*

*Phases run sequenced except where noted. Cha cha for parallel arcs: Clod picks one phase per session, Tony assigns at session start, Clod tags every report-in with the phase name (e.g. "Splice: ready"), Clay can interrupt either arc to park.*

#### Phase Splice ✅ COMPLETE (commit ef2730d, 2026-05-09)

#### Phase Triage — Scoped TAWK autopsy into legacy Tawk.twk (STAGED 2026-05-11, pending build verification — see Immediate above)

#### Phase Port — Callback signature migration

*~50+ `*TawkNow` and `*TawkAct` callbacks change signature to `(PLGparse *state, PLGitem *iTEM)` with children accessed via `iTEM.children["label"]`. Largest single piece of work in the arc. Granularity (one grinding session vs several with design heat) decided when we get there. Splice complete; this is the next phase. **First likely trigger for Phase Lazarus** — callbacks may reference PLGsetParse-the-class.*

- [ ] Inventory the callbacks — count, group by mechanical-vs-design
- [ ] Decide session granularity (Tony + Clay)
- [ ] Port callbacks (one or more Clod sessions)
- [ ] If callbacks reference PLGsetParse → activate Phase Lazarus

#### Phase Compile — First clean build

*New-runtime TAWK builds end-to-end against current support and revised PLG. No errors, no unresolved references. POP: it builds. Depends on Port. **Second likely trigger for Phase Lazarus** — link errors against missing PLGsetParse surface here if Port didn't already need it.*

- [ ] Build new TAWK against current support + revised PLG
- [ ] Resolve unresolved references (TAWK error propagation will surface them in context)
- [ ] If link errors point at PLGsetParse → activate Phase Lazarus
- [ ] Clean compile achieved

#### Phase Lazarus — PLGsetParse revival (STANDBY)

*Three PLGsetParse files (.g + two more, likely .h and .C or .twk) sit in `Parse/BeforeRefactor/`, the local backup directory. They were left behind in the plg refactor. Tonto verified Phase Splice does not need PLGsetParse — the regen body is pure addTest data. But Phase Port (callback bodies) and Phase Compile (link errors) are likely triggers. **Activated on demand, not gating.** When activated, runs in parallel with whatever phase triggered it.*

#### Phase Sandbox — Tests/ verification vs ~/bin/tok

*New TAWK in `Tokf/Tests/` sandbox processes the .twk files in plg/incant/support/tawk repos. Output compared against ~/bin/tok output. **FLOOR POP for the arc.** Depends on Compile.*

#### Phase Promotion — Replace ~/bin/tok

*Sandboxed new TAWK gets promoted to ~/bin/tok. Old binary archived. From here on, all .twk → C++ goes through new TAWK. Depends on Sandbox.*

#### Phase Loop — Self-host POP

*`plg Tawk.g` regenerates Tawk.twk. New TAWK builds from the regenerated source. New TAWK can re-tokkify itself and the ecosystem. **STRETCH POP.** Depends on Promotion.*

---

### PLG — Self-hosting

- [ ] Action blocks feature — `Action actionName { TAWK body } ;` generates `void actionName(PLGparse *state, PLGitem *item) { body }`. Required for self-host loop closure without TEMPORARY BRIDGE.
- [ ] Grammar reorganization — plg.g (structure only), action.g (Action blocks), plgRules.g (shared rules)

### Incant — testByteCode POP

- [ ] Add `Bytecode.mm` to incantGUI Xcode target (manual: drag into project navigator, check target)
- [ ] Emitter rewrite: `gIF` — generate bytecodE attribute instead of old C++ source output
- [ ] Emitter rewrite: `gExpressioN` — same
- [ ] Verify `gBlocK`, `gFOR`, `gWhilE`, `gDO` — emitting bytecodes or old-style C++ strings?
- [ ] Run testByteCode end-to-end — POP: `if righty > 0; maximus = righty * 2;` → `maximus = 26`

---

## 📋 Next Up

### PLG

- [ ] Wash & rinse cycle — architectural ghosts, dead code, old patterns
- [ ] Support/Frame audit — long/int conversions, deprecated functions
- [ ] Xcode workspace (Shape B) — plg + support + tawk + incant properly wired

### TAWK

- [ ] TAWK autopsy remainder (after Phase Triage) — TGV approaching 🚄
  - [ ] Empty comment line context reset fix
  - [ ] `@field` context markers
  - [ ] Include search path support
  - [ ] Unused field warning

### TOK Xcode project — yaml it (+ rename Groups → incant)

*Tonto verified 2026-05-09: TOK.xcodeproj has no project.yml. The plg xcodeproj is yamled (Parse/Revision/project.yml, 195 lines) but TOK is not. Any TOK reorg currently rides along in the .pbxproj binary with no declarative source-of-truth. Reverse-engineering a project.yml from current .pbxproj is non-trivial — plan carefully. Once yamled, the target rename Groups → incant becomes a small project.yml edit.*

- [ ] Investigate: what does a credible TOK project.yml look like? Reference the plg yaml as template
- [ ] Decide on Set B refs (Tawk.twk in Tests, Tawk.regen.twk in Tests, Tawk.h/.C in Links group with JIT siblings — added 2026-05-08 via direct Xcode edit). Are they intentional staging or accidental drift to clean up?
- [ ] Generate project.yml from current .pbxproj
- [ ] Rename target Groups → incant in project.yml
- [ ] Verify `xcodegen generate` produces equivalent project
- [ ] Land project.yml as the source-of-truth going forward

### Cluster C — Buffer 3-arg + new idiom adoption

*Partially landed: parse.rtn has the workaround for the tok post-increment-on-+= bug (commit 0835c34). Other files (Instruct.rtn, GroupRules.mm) still have Cluster C-flavored changes pending. Other instances of the post-increment-on-+= pattern may need similar split-form workarounds.*

- [ ] Sweep for other instances of `... += *p++` or similar post-increment-on-RHS-of-+= constructs across .twk and .rtn files
- [ ] Apply split-form workaround to any found
- [ ] Verify Instruct.rtn changes are commit-ready
- [ ] Commit Cluster C as a coherent unit

### Cluster D — Bytecode gating hook (PARKED)

*The bytecodE → interpret() gating block in GroupRules.mm:786 was hand-edited out of the dirty .mm. Confirmed to be tok-directive-injected debug instrumentation, not source-of-truth content. Re-invokable via groupDirectives when needed.*

- [ ] Revisit when Phase 2 or Phase 3 work resumes

### Cluster E — DEFINing flag / indent-as-structure (HWF Session 4 implementation, IN PROGRESS)

*Implementation for HWF Session 4's settled design (A1 colon-opens-block, B1 single-`;` terminator). Comment handling fixed today; code-block-as-defining-region remains open (Tony's overnight). Once code-block handling lands, full POP against the drawing file.*

- [x] Comment-in-define-block fix to checkSkip (line 159 + 163, in working tree)
- [ ] Code-block-as-defining-region: design context-aware `{` handling
- [ ] Verify against the drawing file (Point + arc + curve + perfRight)
- [ ] Verify no regression in setBrackets or other `[...]`-as-content cases

### Incant

- [ ] `gPrinT` — implement (stub)
- [ ] `gXpress` — implement (stub)
- [ ] `gDeclare` — verify correct
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler — implement when first test needs it
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project — assess InProcess/Bot
- [ ] Distributed GroupItem messaging design

### Sub-goal: Get Clod Fluent in Incant

Clod wrote his first real incant (interpret()) — the demon is emerging.

- [ ] Read `utilities` before next incant write
- [ ] Expand testByteCode with more cases (after gIF/gExpressioN land — see Immediate)

### GUI exploration recon (DEFERRED, by deliberate decision)

*Tony has flagged willingness to unleash Clod-recon on the legacy GUI/ directory as a deliberate exception to the "do not look there" standing rule. Goal: surface what's reusable, build understanding for eventual GUI work. Not today — but when scheduled, this gets its own bounded recon instruction with clear boundaries on what to surface and where findings get captured.*

- [ ] Schedule a session for the bounded GUI recon (Tony's call when)

---

## 🔭 Longer Term (HPDL)

- [ ] Claude as native GroupItem field type (`isCLAUDE`)
- [ ] Incant as distributed virtual OS
- [ ] Go-style channel messaging — Ken Thompson approved theft
- [ ] ZFS-flavored storage — copy-on-write as GroupItem operations
- [ ] Incant display/layout field
- [ ] File system as GroupItems
- [ ] PLG written in Incant
- [ ] Incant self-hosting via JIT
- [ ] Xcode-like development environment written in incant

---

## 🗂️ Housekeeping

- [ ] Add `InProcess/InProcess.xcworkspace` to all CLAUDE.md files
- [ ] plg.g `%%` assumption — document/fix
- [ ] doNotGuard accumulation — Header/CommentPartBoDY/ForwardDecl/Trailer
- [ ] +1000 offset reporting quirk — benign but odd
- [ ] Incant CLAUDE.md drift — says "TAWK runtime replacement Phase 2 is in flight"; bible says "planned, not yet started"; Phase Splice now landed. Both want updating. Settle on accurate language for both.
- [ ] ~/bin/plg dated Nov 2024, pre-day-2 path-resolution fix. Verify installed binary reflects current source, or rebuild and reinstall.
- [ ] Bible safety-rule amendment — line 209 says "never touch Tokf/Tawk.twk before Tests/ proves it". Phase Splice operated directly on Tawk.twk per the runtime-replacement arc design. Amend to clarify Phase 2 arc as the explicit exception.
- [ ] Support repo update process — needs a look. Local support tree drifts ahead of GitHub (Buffer.h, Include/frame: 595 local vs 557 GitHub, possibly others). Hand-maintained tok externals have no systematic sync discipline. Connected to HWF Session 2 (Sign-off ritual) — same root cause as cross-session-boundary verification gaps.
- [ ] Bible update — TAWK Known Issues autopsy table, add item 12: post-increment-dereference (`*p++`) on the RHS of type-aware `+=` triggers infinite recursion in tok's `Instance::setIsUsed()`. Construct in isolation is fine; the combination produces an Instance graph cycle setIsUsed walks without a visited-marker. Workaround: split into two statements (`buffer += *p; p++;`). Bisected to parse.rtn:94 on 2026-05-10.
- [ ] Bible update — add "Clay-designs-Clod-applies-Tony-reviews-before-tok-time" as a named cha cha mode. Used for source changes that pass through a code generator. Staging in a Tests/-sandboxed file gives Tony a fileMerge-shaped review window before any generator runs.
- [ ] Add "out-of-scope directories" standing rule to bible or CLAUDE.md: `Groups/GUI/` is HPDL until GUI work formally opens (deliberate-exception recon possible by Tony's call); `Parse/BeforeRefactor/` is archive. Both should be excluded by default from any recon, grep, or modification.
- [ ] Move Groups/GUI/ to a Reference/ or Stealable/ sibling directory — it's mid-spectrum (stale, will be revised, has stealable parts). Stays in workspace structurally but marked as not-active. Defer until convenient.
- [ ] `git add <file> && git commit` is not equivalent to "commit only `<file>`" if the index already has staged content. Discipline for mirror-style commits across repos: run `git diff --cached --name-only` after `git add` to confirm the index contains only what's intended, before committing.

---

## ✅ Done

### Recent (2026-05)

- [x] **Phase Triage staged in Tokf/Tests/FormatC.twk (2026-05-11)** — three edits: GC inheritance in C++ class declarations, guardTokenFromName helper, close() emits include-guard prologue + `#endif`. Live Tokf/FormatC.twk untouched. Awaiting Tony's fileMerge review and tomorrow's build/test/promotion.
- [x] **checkSkip comment-in-define-block fix (2026-05-11)** — line 159: `atContent += 2` → `atContent++`; line 163: `atRuleMark = atContent + 1`. Fix bisected via xcode debugger walk-through with Tony driving. Comment processing was over-advancing past the closing `\n` by 1, leaving indent state stale; fix lets the outer loop's tail `atContent++` complete the advancement so the newline handler fires correctly. Working tree, pending commit.
- [x] **Xcode dev loop working (2026-05-11)** — Tony got the build clean overnight. Build target "Groups" compiles, errors actionable in IDE, debugger usable.
- [x] **PLGset.addTest() removed (2026-05-11)** — was never called. Stashed in Parse/BeforeRefactor/ for safe keeping.
- [x] RuleStuff fix (commit 0835c34, 2026-05-10) — parse.rtn:94 split-form workaround for tok post-increment-on-+= bug.
- [x] Cluster B regen + revert (commit 6398920, 2026-05-10) — regenerated four .mm/.h pairs from .twk source-of-truth, reverted XML/WorkingOn/generate's PLGset → CharSet mis-edit.
- [x] Move five working files (commit fec9358, 2026-05-10) — Testing.twk, action.twk, Test.twk, parts.twk, Simple.twk moved from Groups/ to Parse/BeforeRefactor/.
- [x] Buffer source-of-truth verified (2026-05-10) — Buffer.twk consistently 3-arg.
- [x] Phase Splice complete (commit ef2730d, 2026-05-09) — setRules() body in Tokf/Tawk.twk replaced with new-format body from Tawk.regen.twk.
- [x] PLG `process()` CWD-relative path contract — drop sourceDir-from-filename, basename-only outFile (commit da51193, day 2)
- [x] Bible May 7 polishes — resurrection-reader standard, day-3 inverse-failure lesson, path-bug entry, eRocka removed, TOK xcode references dropped
- [x] Bible mirror sweep across all four repos (plg → support → tawk → incant) — same MD5 across all four
- [x] Incant repo cleanup commit (b5375e8) — Maps/ deletions + symlink, BackupXML/ removed, XML/.DS_Store untracked, XML/junk removed, Tests/ added to .gitignore, XML/Notions/flags updated
- [x] HWF.md trim ritual added; Session 1 lessons captured day-3 recursion + inverse failure
- [x] Verification protocol added — fetch from GitHub raw with cache-busting (commit-pinned URL or `?v=` query) after every push

### Earlier

- [x] Four repos created and public (plg, incant, support, tawk)
- [x] CLAUDE.md, TODO.md, projectBible.md present in all four repos (each repo's content is repo-focused, bible content mirrored)
- [x] "What Is Incant?" wiki page
- [x] Support static library (libsupport.a)
- [x] PLGset moved to support repo
- [x] foundIn dependency cycle resolved
- [x] addTest() implemented, setRules() 48% smaller
- [x] Guards implemented — setGuard() howitzer fix (8 cases, null vs empty)
- [x] .next chain merges, Alternative.match optional element fix
- [x] Two-table process() pattern
- [x] Full callback chain: RuleplgNow, AlternativeplgAct, ElementplgAct, ElementTypeplgAct
- [x] PLGitem.runDeferred() implemented
- [x] Testing.g parses end-to-end, labels work
- [x] plg.g parses end-to-end — 39 rules
- [x] Tawk.g parses end-to-end — 200 rules captured, 177 populated
- [x] Generator rewrite — addTest format, 76% smaller
- [x] banged/noSkip fix — 6 coordinated changes
- [x] noSkip propagation through `}&` modifier
- [x] Comment+CommentBody decomposition
- [x] Pre-parse comment stripping — stripComments() in PLGparse
- [x] Grammar source tracked in Parse/Revision/Grammar/
- [x] APFS case collision resolved — Grammar/ subdir
- [x] plg.g self-describes — Start/Header/Include fixed
- [x] Bare-include over-matching investigated — STALE FRAMING, was never actually broken
- [x] TAWK Directives documented and used in anger
- [x] Xcode project rebuilt via xcodegen/project.yml (plg only — TOK still unyamled, see Next Up)
- [x] Session docs created and tracked
- [x] Bytecode design decisions settled
- [x] interpret() written in incant — Clod's first real incant
- [x] bcOPs registry + C++ handlers (Bytecode.mm)
- [x] Gating hook in GroupRules.mm:786 (bytecodE → interpret())
- [x] All 4 repos updated with bytecode section
