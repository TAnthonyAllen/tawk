# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up (2026-05-14)

**First work — bible refresh.** Tony reviewed `bible-amendments.md` overnight (it lives in his Downloads). Draft the full revised projectBible.md incorporating Tony's edits to the four amendments:

1. New section: "Out-of-Scope Directories" (slots after Local Directory Map)
2. Phase 2 safety rules clarification (Phase Splice + Triage as explicit exceptions)
3. New section: "TAWK Known Issues — Autopsy Table" (slots before Resolved Issues Log; items 1-6 sourced from older bible Priority Plan, need Tony's confirmation; item 12 is the tok post-increment-on-+= bug)
4. Expanded Working Relationship section (Cha Cha Modes subsection + Working Disciplines subsection)

Then mirror across all four repos.

**Other threads ready to advance:**

- **PLGset issues.** Tony testing PLGset against fixed Buffer overnight to see whether the 538976288 setBuffer-size issue resolves automatically. If it doesn't, a focused conversation on PLGset's remaining issues. If it does, one less thing to chase.

- **plg directory flatten.** Tonto's recon already done (see Phase Integrate prep section below). Tomorrow opens with: verify PLGrgx/PLGtester usage paths to confirm the recon's coupling-list interpretation, then sequence the move. Real piece of work, possibly ~2-3 hours if smooth.

- **Phase Integrate work.** Today's Triage promotion failed at build because Instance.C uses old Buffer/PLGitem surfaces. That's Phase Integrate's whole job. Migration recon, then file-by-file migration, then TOK builds, then tokTemp, then Phase Triage validates, then ~/bin/tok promotion. Bigger arc than fits in any one day.

- **Commit decisions for accumulated working-tree drift.** Today committed Buffer migration; yesterday's PLGset/CharSet rewrite still uncommitted, plus various Cluster C and older drift across multiple files. Worth a fresh-eyes pass on what should land as commits and what should be reverted.

**Standing wake-up practice (new 2026-05-13):**
After reading the docs, Clod runs `git diff --stat HEAD` in each repo to surface what's changed in working source since last commit. Reports the summary; Tony fills in context for anything significant. This is now part of the regular wake-up flow.

**Out of scope (do not browse, survey, or modify):**
- `Groups/GUI/` — HPDL legacy GUI material
- `Parse/BeforeRefactor/` — archive
- `Tokf/Backed/`, `Tokf/BeforeRefactor/`, `Tokf/OldStuff/` — TAWK archives
- Any `Aside/`, `BackupIncludes/`, `*.rvsd` — accumulated backups

**Known current state:**
- TOK build is broken on Tokf/Instance.C (old Buffer + PLGitem surface). This is Phase Integrate work.
- Yesterday's PLGset/CharSet rewrites still uncommitted (in working tree).
- Tokf/FormatC.twk live source carries today's Phase Triage promotion (uncommitted; binary identical to vetted Tests/ version).
- Various accumulated drift across plg class set (May 8-10) and support files (May 8-10) — pre-existing, not from today.
- Today's Buffer migration committed (today's commits).

---

## 🔥 Immediate (current sprint)

### Phase Integrate — Tokf migration to new plg

*Today's Phase Triage promotion exposed the gap: TOK build fails because Tokf source (Instance.C, others) uses old Buffer + PLGitem surfaces that have been migrated everywhere except Tokf. Until Tokf migrates, new TOK can't compile. Until new TOK compiles, Phase Triage's edits can't be runtime-validated. Until Phase Triage is validated, ~/bin/tok can't be promoted.*

*Replaces and absorbs Phase Port (callback signatures) and Phase Compile (first clean build). The work is bigger than Port alone — Port was about callbacks; Integrate is about every Tokf source that uses old plg/Buffer/PLGitem/PLGset surfaces migrating to the new ones.*

- [ ] **Recon:** survey Tokf/ for files referencing Buffer, PLGitem, PLGset, PLGtester. Categorize by migration shape (mechanical Buffer 3-arg, PLGitem call-site refactor, PLGtester elimination).
- [ ] **Sequence:** pick a small leaf-like Tokf file to migrate first as proof. Not Instance.C — too central.
- [ ] **Migrate file by file.** Each migration its own commit. Run TOK build after each — even if it still fails, error count should drop. PLGtester gets parked in Parse/Backup/ as the migration proceeds (its files only needed by old tok runtime).
- [ ] **Reach compilation.** Build clean — Phase Compile complete.
- [ ] **Build ~/bin/tokTemp** (not ~/bin/tok — distinct binary to avoid confusion during transition). Old ~/bin/tok stays in place as safety net.
- [ ] **Smoke test (Phase Sandbox).** Run tokTemp against representative .twk corpus. Diff output against current expectations.
- [ ] **Phase Triage validation.** Verify tokTemp output has include guards + `#include "gc/gc_cpp.h"` + `: public gc` for C++ classes.
- [ ] **Phase Promotion.** When validated, ~/bin/tok ← tokTemp. Old tok archived.

### Phase Triage — promoted to live source, blocked on Phase Integrate

*FormatC.twk has been promoted from Tests/ to live Tokf/FormatC.twk (uncommitted in working tree, MD5 882ea729454d29870d07238b799e5055). Three edits: GC inheritance in C++ class declarations, guardTokenFromName helper, close() emits include-guard prologue + `#endif`. Live source is the source-of-truth. Runtime validation waits on Phase Integrate.*

- [x] Recon, design, sandbox edits, fileMerge review, approval
- [x] Promote Tests/FormatC.twk to live Tokf/FormatC.twk (2026-05-13)
- [ ] Commit (deferred — sits in working tree pending Phase Integrate completion)
- [ ] Runtime-validate via tokTemp once Phase Integrate produces a working binary

### Buffer migration — committed today

*Replaced bufferFactory{1,2,3,4} free functions with three real C++ constructors. Three-step migration: edit Buffer.twk (add ctors, remove factories), edit Include/frame (add ctor decls, remove `new bufferFactory` alias, remove separate `external Buffer.h` block), re-tok consumers. The setBuffer-size 538976288 crash from yesterday should be addressed by the explicit field zero-init that tok now emits in constructor bodies (vs. relying on calloc's implicit zeroing in factories).*

- [x] Rewrite Buffer.twk with three constructors
- [x] Edit Include/frame external block
- [x] Drop unused `include plg.ext` from frameIncludes (verified safe across all support consumers)
- [x] Re-tok Buffer.twk → fresh Buffer.{C,h}
- [x] Re-tok 3 consumer .twk files (CharSet, GroupControl, GroupRules) — all clean
- [x] Tokf-side consumers skipped (blocked on Phase Integrate)
- [x] plg-side consumers (Parse/Revision/PLG, PLGparse) deferred — not in today's scope
- [x] Commit today (Buffer migration + Cluster C 3-arg method adoption bundled)
- [ ] PLGset re-test against fixed Buffer (Tony overnight)

### PLGset / CharSet — yesterday's rewrite, needs commit

*Yesterday's representation rewrite (bit-packed `map[4]` → `unsigned char *inSet`) is in working tree, untested beyond compile-and-run. Build is clean and the morning corruption symptom is gone. PLGset's 538976288 setBuffer-size issue surfaced afterward; today's Buffer migration may address it. Commit decision waits on PLGset re-test against fixed Buffer.*

- [x] Rewrite PLGset.twk with `unsigned char *inSet` storage
- [x] Rewrite CharSet.twk with parallel representation
- [x] Update `support/Include/frame` external for CharSet
- [ ] Tony tests PLGset against fixed Buffer (overnight)
- [ ] Commit decision based on test result

### checkSkip — dedent for defining mode

*Pinned from 2026-05-12: the dedent-half of the `defining` branch in checkSkip's indent-check is commented out — `//or defining { *atContent = ';'; }` at line ~195 of GroupRules.twk. When `:`-opened blocks dedent, no synthetic close fires, and the block doesn't close. Tony has been working on checkSkip indentation handling after-hours.*

- [ ] Continue Tony's overnight design work
- [ ] Decide token shape for defining-mode dedent
- [ ] Apply fix to checkSkip
- [ ] Verify against generator define block in generate file (POP: `generatE` becomes peer of `generator` instead of being absorbed)

### checkSkip — code-block-as-defining-region (Tony's continued overnight)

*Open since 2026-05-11. Inside `{ ... }` code blocks within a defining context, checkSkip processes the contents and mutates indent state. Yesterday's `defineSkipSet` approach broke `setBrackets`. Tony continues design work.*

- [ ] Continue Tony's overnight design
- [ ] Apply, test against drawing file (Point + arc + curve)
- [ ] Verify no regression in setBrackets

### plg directory flatten — recon complete, action deferred to tomorrow

*Tonto recon completed 2026-05-13. Plan: move Parse/Revision/* up to Parse/, move legacy Parse/* contents to Parse/Backup/. Wrinkles surfaced:*

- **Case-insensitivity collision** between legacy `plg.{C,h,twk}` and Revision `PLG.{C,h,twk}` — 12 file overlap (case-insensitive). Must sequence: legacy files OUT first, then Revision UP.
- **`.git` lives in Parse/Revision/** — needs to move with the contents (`mv Revision/.git Parse/.git`). Parse/ itself is not a git repo currently.
- **PLGrgx is active source** despite living in legacy Parse/ — referenced from incant (GroupItem, GroupBody, GroupRules) and support/Include (groups.ext, PLGrevision). Should move with Revision content, not to Backup. Tony confirmed: we need/will use PLGrgx.
- **PLGtester is dead** — used only by old tok being phased out. Park in Parse/Backup/. Will be removable when Phase Integrate completes.
- **plgDirectives in plg.xcodeproj** — references will need path updates post-move.
- **References to "Parse/Revision" across docs** — 4 each in projectBible.md (in 4 repos), TODO.md (in 4 repos), and Sessions notes. Mechanical doc-update sweep post-move.

Tomorrow's sequence:
- [ ] Verify PLGrgx/PLGtester usage paths (confirms recon's interpretation)
- [ ] Move legacy Parse/* contents to Parse/Backup/ (preserves them, makes way for Revision contents)
- [ ] Move Parse/Revision/* up to Parse/ (case collisions now resolved by step above)
- [ ] Move Parse/Revision/.git to Parse/.git
- [ ] Update plg.xcodeproj/project.yml path references (or regenerate via xcodegen)
- [ ] Update doc references across four repos
- [ ] Verify plg repo still functional (build, push)

### Bible refresh — first thing tomorrow

*See "Tomorrow's wake-up" above.*

### TAWK — Back Up and Running

#### Phase Splice ✅ COMPLETE (commit ef2730d, 2026-05-09)

#### Phase Triage — promoted to live source (see Immediate above)

#### Phase Integrate — see Immediate (replaces Phase Port + Phase Compile)

#### Phase Lazarus — PLGsetParse revival (STANDBY)

#### Phase Sandbox — Tests/ verification vs ~/bin/tok (folded into Phase Integrate)

#### Phase Promotion — Replace ~/bin/tok (folded into Phase Integrate)

#### Phase Loop — Self-host POP

*`plg Tawk.g` regenerates Tawk.twk. New TAWK builds from regenerated source. Stretch POP.*

---

### PLG — Self-hosting

- [ ] Action blocks feature
- [ ] Grammar reorganization — plg.g (structure only), action.g (Action blocks), plgRules.g (shared rules)

### Incant — testByteCode POP

- [ ] Add `Bytecode.mm` to incantGUI Xcode target
- [ ] Emitter rewrite: `gIF` — generate bytecodE attribute
- [ ] Emitter rewrite: `gExpressioN` — same
- [ ] Verify `gBlocK`, `gFOR`, `gWhilE`, `gDO`
- [ ] Run testByteCode end-to-end — POP: `if righty > 0; maximus = righty * 2;` → `maximus = 26`

---

## 📋 Next Up

### PLG

- [ ] Wash & rinse cycle
- [ ] Support/Frame audit
- [ ] Xcode workspace (Shape B)

### TAWK

- [ ] TAWK autopsy remainder (after Phase Integrate)

### TOK Xcode project — yaml it (+ rename Groups → incant)

### Cluster C — Buffer 3-arg + new idiom adoption ✅ effectively complete

*Today's Buffer migration included re-toking Cluster C-affected files. The 3-arg method signatures (appendChar/appendInt/appendString) are now in regenerated artifacts via the same commit as the constructor migration.*

### Cluster D — Bytecode gating hook (PARKED)

### Cluster E — DEFINing flag / indent-as-structure (IN PROGRESS)

*Comment handling fixed 2026-05-11. Code-block handling open. Dedent-half of defining branch needs implementation. Tony continues overnight design work.*

### Incant

- [ ] `gPrinT` — implement (stub)
- [ ] `gXpress` — implement (stub)
- [ ] `gDeclare` — verify correct
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project — assess InProcess/Bot
- [ ] Distributed GroupItem messaging design

### Sub-goal: Get Clod Fluent in Incant

### GUI exploration recon (DEFERRED, by deliberate decision)

### Maps → move to support source (Tony's note, deferred)

### TOK build machinery lives outside all four GitHub repos

*Per today's recon: `~/data/InProcess/TOK/TOK.xcodeproj` is the build mechanism. No project.yml, no git tracking. Real exposure — if the build setup gets damaged, no remote backup. Worth eventual housekeeping.*

---

## 🔭 Longer Term (HPDL)

- [ ] Claude as native GroupItem field type (`isCLAUDE`)
- [ ] Incant as distributed virtual OS
- [ ] Go-style channel messaging
- [ ] ZFS-flavored storage
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
- [ ] +1000 offset reporting quirk
- [ ] Incant CLAUDE.md drift — settle accurate language now Phase Splice has landed
- [ ] ~/bin/plg dated Nov 2024 — verify or rebuild
- [ ] Support repo update process — needs a look (drift between local and GitHub)
- [ ] **PLGset/CharSet architectural note** (stashed bible amendment item 5 — pending decision on whether it warrants bible status)
- [ ] **Phase Integrate detailed plan** (stashed bible amendment item 6 — landed in TODO Immediate above; bible amendment can come when phase is more concrete)
- [ ] Move Groups/GUI/ to a Reference/ or Stealable/ sibling directory
- [ ] Move Groups/Maps/ to support source
- [ ] Accumulated working-tree drift across plg class set (May 8-10), support Buffer/OCroutines/StringRoutines (May 8-10), KeyTable bulk-touch (May 8 12:12) — sort what should commit vs revert

---

## ✅ Done

### Recent (2026-05)

- [x] **Buffer migration to constructors (2026-05-13)** — replaced bufferFactory{1,2,3,4} free functions with three real C++ constructors (Buffer(), Buffer(name), Buffer(name, size)). Edits: Buffer.twk (add ctors, remove factories), Include/frame (add ctor decls, remove `new bufferFactory` alias, remove `external Buffer.h` block), frameIncludes (drop unused plg.ext include). Re-tok'd Buffer, CharSet, GroupControl, GroupRules. Tokf-side consumers (Instance, FormatC, Tawk) skipped pending Phase Integrate. plg-side consumers deferred. Bundled with Cluster C 3-arg method adoption in same commit.
- [x] **Phase Triage promoted to live source (2026-05-13)** — Tokf/Tests/FormatC.twk → Tokf/FormatC.twk. MD5 882ea729454d29870d07238b799e5055. Sits uncommitted in working tree; runtime validation waits on Phase Integrate.
- [x] **Include/changes restore (commit 17982d2, 2026-05-13)** — restored file inadvertently swept into commit 604ad09 by glance-and-proceed on staged deletion. Discipline lesson filed.
- [x] **TODO mirror push for 2026-05-12 (commits 7707d2b incant, f6c27ff plg, 06992d2 tawk, 604ad09 support, 2026-05-13)** — yesterday's end-of-session TODO finally landed on all four repos.
- [x] **PLGset / CharSet rewrite (2026-05-12)** — bit-packed `map[4]` → `unsigned char *inSet`. Trivially debuggable. Working tree, awaiting test against fixed Buffer.
- [x] **Phase Triage staged & approved (2026-05-12)** — three edits to FormatC.twk: GC inheritance, guardTokenFromName helper, close() with guard emission.
- [x] **checkSkip comment-in-define-block fix (commit a219689, 2026-05-11)**
- [x] Xcode dev loop working (2026-05-11)
- [x] PLGset.addTest() removed (2026-05-11)
- [x] RuleStuff fix (commit 0835c34, 2026-05-10)
- [x] Cluster B regen + revert (commit 6398920, 2026-05-10)
- [x] Move five working files (commit fec9358, 2026-05-10)
- [x] Buffer source-of-truth verified (2026-05-10)
- [x] Phase Splice complete (commit ef2730d, 2026-05-09)
- [x] PLG `process()` CWD-relative path contract (commit da51193)
- [x] Bible May 7 polishes
- [x] Bible mirror sweep across all four repos
- [x] Incant repo cleanup commit (b5375e8)
- [x] HWF.md trim ritual added
- [x] Verification protocol added

### Earlier

- [x] Four repos created and public (plg, incant, support, tawk)
- [x] CLAUDE.md, TODO.md, projectBible.md present in all four repos
- [x] "What Is Incant?" wiki page
- [x] Support static library (libsupport.a)
- [x] foundIn dependency cycle resolved
- [x] addTest() implemented, setRules() 48% smaller
- [x] Guards implemented — setGuard() howitzer fix
- [x] .next chain merges, Alternative.match optional element fix
- [x] Two-table process() pattern
- [x] Full callback chain
- [x] PLGitem.runDeferred() implemented
- [x] Testing.g parses end-to-end, labels work
- [x] plg.g parses end-to-end — 39 rules
- [x] Tawk.g parses end-to-end — 200 rules captured, 177 populated
- [x] Generator rewrite — addTest format, 76% smaller
- [x] banged/noSkip fix
- [x] noSkip propagation through `}&` modifier
- [x] Comment+CommentBody decomposition
- [x] Pre-parse comment stripping
- [x] Grammar source tracked in Parse/Revision/Grammar/
- [x] APFS case collision resolved
- [x] plg.g self-describes
- [x] Bare-include over-matching investigated
- [x] TAWK Directives documented and used in anger
- [x] Xcode project rebuilt via xcodegen/project.yml (plg only)
- [x] Session docs created and tracked
- [x] Bytecode design decisions settled
- [x] interpret() written in incant — Clod's first real incant
- [x] bcOPs registry + C++ handlers (Bytecode.mm)
- [x] Gating hook in GroupRules.mm:786
- [x] All 4 repos updated with bytecode section
