# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up (next session — may be later today after recess)

**First work — Tony fixes the double-define bug.** Offline work. Should be short. The bug: testCodE parses correctly through aCTionCodE, but afterward the parse continues in a way that causes the field to be defined a second time. Hypothesis: testAction's two paths in parse.rtn — one for parseACTION/no-label, one for the older label-passed pattern — may both fire under some conditions, or the upstream parse loop misreads the post-parseAction state. Single-step through the debugger from just after aCTionCodE returns true.

**After Tony's fix lands, then session resumes for deferred tasks.**

**Deferred tasks (pick order in next session):**

- **plg directory flatten** — Tonto recon complete (see notes below). Move legacy Parse/* contents to Parse/Backup/, move Parse/Revision/* up to Parse/. Wrinkles: case-insensitivity collision (must sequence legacy-out before Revision-up), .git relocation, PLGrgx is active source despite living in legacy Parse/, PLGtester is dead-pending-Phase-Integrate (park in Parse/Backup/), plgDirectives in plg.xcodeproj needs path updates, doc references across 4 repos.

- **PLGset "put in its place"** — Content commit landed (today). Location question: PLGset stays in Parse/ alongside the rest of plg's new architecture after the flatten. The flatten task covers this.

- **Phase Integrate** — Recon Tokf/ for files using old Buffer/PLGitem/PLGset/PLGtester surfaces. Bigger arc; see Phase Integrate section below.

- **Bible refresh** — Tony reviews bible-amendments.md (in Downloads). Draft full bible incorporating Tony's edits. Mirror across all four repos.

**Reading targets:**
- `Groups/grammar` (the file Tony pasted today) for CodE/DatA bootstrap rules
- `Groups/GroupMain.twk` (bootstrapper changes for CodE/DatA)
- `Groups/RuleStuff.twk` and `Groups/parse.rtn` (testAction change)
- `Groups/ruleActions.rtn` (aCTionCodE method)
- `Groups/GroupItem.twk` and `Groups/GroupRules.twk` (other overnight changes)

**Standing wake-up practice:**
After reading the docs, Clod runs `git diff --stat HEAD` in each repo to surface what's changed in working source since last commit. Tony fills in context. This is part of the regular wake-up flow now.

**Out of scope:**
- `Groups/GUI/`, `Parse/BeforeRefactor/`, archive directories

**Known current state:**
- Today's overnight work committed (CodE/DatA parseAction approach in 5 incant files + grammar)
- Yesterday's PLGset/CharSet rewrite committed today (was uncommitted in working tree until today's session)
- Phase Triage FormatC.twk live source still uncommitted (waits on Phase Integrate)
- Other accumulated drift from prior sessions still in working tree (Cluster C remnants, KeyTable May 8 bulk-touch, etc.)
- POP partial: incant loads, first definition (testCodE) parses through aCTionCodE successfully, then incant bails silently after a double-define. Tony's after-hours fix target.
- Xcode was updated overnight. Lesson filed: when Xcode does an update mid-project, **Clean Build Folder before anything else** when weird build/runtime behavior appears.

---

## 🔥 Immediate (current sprint)

### CodE/DatA parseAction approach — committed today

*Yesterday's checkSkip indent-as-structure problem (code blocks inside define mode mutating indent state) solved by going outside checkSkip entirely. New grammar rules CodE and DatA recognize `{ ... }` field values as atomic spans via parseAction mechanism — checkSkip never sees inside the code block.*

*Implementation: CodE is a parseAction rule with `{` and `}` as left/right attributes. aCTionCodE method scans the input from atRuleMark forward to find the delimiters and sets the label's token to the captured span. The parseAction modifier on the rule causes testAction (in parse.rtn) to dispatch instead of standard match logic. Existing parseAction infrastructure was dormant since the GroupItem parse rewrite; testAction needed a small fix to pass the rule (not the label) to aCTionCodE.*

- [x] Add CodE rule to grammar (bootstrap section + grok registry definition in GroupMain.twk)
- [x] Add CodE to DatA alternation in grammar (so field values can be code blocks)
- [x] Fix testAction in parse.rtn to dispatch correctly for parseACTION rules
- [x] aCTionCodE method in ruleActions.rtn
- [x] All five touched .twk files re-tok'd to current .C/.mm artifacts
- [x] Commit overnight work
- [ ] **Tony's overnight fix:** double-define of testCodE after parseAction returns. Hypothesis: testAction's two dispatch paths may both fire, or upstream parse loop misreads post-parseAction state. Debug session needed.

### PLGset / CharSet — committed today (yesterday's rewrite)

*Yesterday's representation rewrite (bit-packed `map[4]` → `unsigned char *inSet`, 256 bytes, debuggable) sat uncommitted overnight. Today's POP validated the rewrite works through 5 successive PLGset instantiations in bootstrapper (the 6th caused trouble but turned out to be an unrelated Xcode-update-related stale build that resolved with clean build). Committed today.*

- [x] Rewrite PLGset.twk with `unsigned char *inSet` storage
- [x] Rewrite CharSet.twk with parallel representation
- [x] Updated `support/Include/frame` external for CharSet
- [x] Validated through POP (first 5 PLGset instantiations work cleanly post-Buffer-migration)
- [x] Commit

### Phase Integrate — Tokf migration to new plg

*The big arc. Today's POP exposes that incant runtime is workable enough to make progress on Phase Integrate. Approach: recon Tokf/ for old-API references, migrate file-by-file, build ~/bin/tokTemp, validate Phase Triage, eventually promote to ~/bin/tok.*

- [ ] **Recon:** survey Tokf/ for files referencing Buffer, PLGitem, PLGset, PLGtester. Categorize by migration shape.
- [ ] **Sequence:** pick a small leaf-like Tokf file to migrate first as proof.
- [ ] **Migrate file by file.** Each migration its own commit. Run TOK build after each.
- [ ] **Reach compilation.** Build clean.
- [ ] **Build ~/bin/tokTemp** (not ~/bin/tok — distinct binary).
- [ ] **Smoke test (Phase Sandbox).** Run tokTemp against representative .twk corpus.
- [ ] **Phase Triage validation.** Verify tokTemp output has include guards + `#include "gc/gc_cpp.h"` + `: public gc`.
- [ ] **Phase Promotion.** When validated, ~/bin/tok ← tokTemp.

### Phase Triage — promoted to live source, blocked on Phase Integrate

*FormatC.twk promoted from Tests/ to live Tokf/FormatC.twk (uncommitted, MD5 882ea729454d29870d07238b799e5055). Runtime validation waits on Phase Integrate.*

- [x] All design and staging work
- [x] Promote Tests/FormatC.twk to live Tokf/FormatC.twk
- [ ] Commit (deferred — sits in working tree pending Phase Integrate completion)
- [ ] Runtime-validate via tokTemp once Phase Integrate produces a working binary

### plg directory flatten — recon complete, action deferred

*Tonto recon completed 2026-05-13. Plan: move Parse/Revision/* up to Parse/, move legacy Parse/* contents to Parse/Backup/. Wrinkles surfaced:*

- **Case-insensitivity collision** between legacy `plg.{C,h,twk}` and Revision `PLG.{C,h,twk}` — 12 file overlap. Must sequence: legacy files OUT first, then Revision UP.
- **`.git` lives in Parse/Revision/** — needs `mv Revision/.git Parse/.git`. Parse/ itself is not a git repo currently.
- **PLGrgx is active source** despite legacy location — referenced from incant (GroupItem, GroupBody, GroupRules) and support/Include (groups.ext, PLGrevision). Should move with Revision content, not to Backup.
- **PLGtester is dead** — used only by old tok being phased out. Park in Parse/Backup/.
- **plgDirectives in plg.xcodeproj** — references need path updates post-move.
- **References to "Parse/Revision" across docs** — 4 each in projectBible.md (in 4 repos), TODO.md (in 4 repos), Sessions notes. Mechanical doc-update sweep post-move.

Sequence:
- [ ] Verify PLGrgx/PLGtester usage paths
- [ ] Move legacy Parse/* contents to Parse/Backup/ (preserves them, makes way for Revision contents)
- [ ] Move Parse/Revision/* up to Parse/
- [ ] Move Parse/Revision/.git to Parse/.git
- [ ] Update plg.xcodeproj/project.yml path references
- [ ] Update doc references across four repos
- [ ] Verify plg repo still functional (build, push)

### Bible refresh — deferred

*Bible amendments file lives in Tony's Downloads. Tony reviews when ready (was deferred from yesterday and today). When ready: Clay drafts full revised projectBible.md incorporating Tony's edits, Clod mirrors across all four repos.*

- [ ] Tony reviews bible-amendments.md
- [ ] Clay drafts full revised bible
- [ ] Clod mirrors across four repos

### TAWK — Back Up and Running

#### Phase Splice ✅ COMPLETE (commit ef2730d, 2026-05-09)

#### Phase Triage — promoted to live source (see Immediate above)

#### Phase Integrate — see Immediate (replaces Phase Port + Phase Compile)

#### Phase Lazarus — PLGsetParse revival (STANDBY)

#### Phase Loop — Self-host POP

---

### PLG — Self-hosting

- [ ] Action blocks feature
- [ ] Grammar reorganization

### Incant — testByteCode POP

- [ ] Add `Bytecode.mm` to incantGUI Xcode target
- [ ] Emitter rewrite: `gIF` — generate bytecodE attribute
- [ ] Emitter rewrite: `gExpressioN` — same
- [ ] Verify `gBlocK`, `gFOR`, `gWhilE`, `gDO`
- [ ] Run testByteCode end-to-end

---

## 📋 Next Up

### PLG

- [ ] Wash & rinse cycle
- [ ] Support/Frame audit
- [ ] Xcode workspace (Shape B)

### TAWK

- [ ] TAWK autopsy remainder (after Phase Integrate)

### TOK Xcode project — yaml it

### Cluster C — Buffer 3-arg + new idiom adoption ✅ effectively complete

### Cluster D — Bytecode gating hook (PARKED)

### Cluster E — DEFINing flag / indent-as-structure (IN PROGRESS)

*Code-block-as-defining-region addressed via CodE/DatA parseAction (today). Dedent-half of defining branch still open from earlier sessions. Tony continues design work.*

### Incant

- [ ] `gPrinT` — implement (stub)
- [ ] `gXpress` — implement (stub)
- [ ] `gDeclare` — verify correct
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project — assess InProcess/Bot
- [ ] Distributed GroupItem messaging design

### GUI exploration recon (DEFERRED, by deliberate decision)

### Maps → move to support source

### TOK build machinery lives outside all four GitHub repos

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
- [ ] doNotGuard accumulation
- [ ] +1000 offset reporting quirk
- [ ] Incant CLAUDE.md drift
- [ ] ~/bin/plg dated Nov 2024 — verify or rebuild
- [ ] Support repo update process — needs a look
- [ ] PLGset/CharSet architectural note (stashed bible amendment)
- [ ] Move Groups/GUI/ to a Reference/ sibling directory
- [ ] Move Groups/Maps/ to support source
- [ ] Accumulated working-tree drift sort: what should commit vs revert
- [ ] **Xcode-update discipline:** when Xcode does an update, **Clean Build Folder before debugging any weird runtime behavior**. Stale derived data can produce phantom bugs.

---

## ✅ Done

### Recent (2026-05)

- [x] **CodE/DatA parseAction approach (2026-05-14)** — grammar change to handle `{ ... }` field values atomically, sidestepping checkSkip's indent-state corruption issue from prior sessions. Touched: grammar, GroupMain.twk (bootstrapper), RuleStuff.twk, parse.rtn (testAction fix), ruleActions.rtn (aCTionCodE), GroupItem.twk, GroupRules.twk.
- [x] **PLGset/CharSet rewrite committed (2026-05-14)** — yesterday's representation rewrite was uncommitted; today's POP validated and committed.
- [x] **Buffer migration to constructors (2026-05-13)** — bufferFactory{1,2,3,4} → three real C++ constructors. Edits across Buffer.twk, Include/frame, frameIncludes; re-tok'd Buffer, CharSet, GroupControl, GroupRules.
- [x] **Phase Triage promoted to live source (2026-05-13)** — Tokf/Tests/FormatC.twk → Tokf/FormatC.twk. Uncommitted pending Phase Integrate.
- [x] **Include/changes restore (commit 17982d2, 2026-05-13)** — restored file inadvertently swept into commit 604ad09.
- [x] **TODO mirror push (commits 7707d2b/f6c27ff/06992d2/604ad09, 2026-05-13)** — yesterday's TODO finally landed on all four repos.
- [x] **PLGset / CharSet representation rewrite (2026-05-12)** — bit-packed `map[4]` → `unsigned char *inSet`.
- [x] **Phase Triage staged & approved (2026-05-12)** — three edits to FormatC.twk.
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

[unchanged]
