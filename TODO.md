# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up

**First work — Tony's checkSkip double-define investigation.** Offline target. From earlier today's debugging: testCodE parses correctly through aCTionCodE, but the parser then rewinds back to the original cursor position and re-parses the same field, producing a duplicate definition. The synthetic `:` insertion in checkSkip's indent-mode handling appears to be the source — checkSkip is inserting a `:` where it shouldn't, the parser sees corrupted input, rewinds and replays. Possibly the same surface as 2026-05-11's comment-fix area, just for a different trigger. Tony's after-hours debugger work.

**After Tony's fix, session resumes for deferred tasks. Pick from:**

- **Bible refresh.** Tony reviewed bible-amendments.md, approved with caveat "may have glossed details." Clay drafts full revised projectBible.md incorporating Tony's edits AND today's flatten (directory map needs rewriting post-flatten, not just substitution — flatten landed mid-day so amendments file doesn't reflect it). Clod mirrors across four repos.

- **Phase Integrate kickoff.** Recon Tokf/ for files using old Buffer/PLGitem/PLGset/PLGtester surfaces. Migrate file-by-file, build ~/bin/tokTemp. The big arc for the multi-week future.

- **plg xcode link cleanup + yaml refresh.** Today's flatten left some broken file-references in the plg xcodeproj (PLGrevision, plgDirectives, Include-side tok externals). Tony will manually fix these in Xcode UI, then we yaml-refresh project.yml to make it source-of-truth for the post-flatten state. Low-priority cosmetic work; build is fine without it.

- **documentation.md surfaces.** Tony's WIP file in plg root. Conversation-worthy when it comes up again.

- **TOK Xcode yaml-from-scratch.** TOK.xcodeproj has no project.yml and lives outside all four GitHub repos. Reverse-engineering from existing .pbxproj is the work. May also benefit from rename (Groups target → incant). On housekeeping for whenever.

**Reading targets:**
- `Parse/` is now the plg repo root (formerly Parse/Revision/). All plg source lives here directly now.
- `Parse/Backup/` holds parked legacy plg material (gitignored)
- `Parse/plgDirectives` is the active debugging-directive file (untracked, kept in place)
- `support/Frame/PLGset.{twk,C,h}` and `support/Frame/CharSet.{twk,C,h}` — both sisters live here as of 2026-05-14

**Standing wake-up practice:**
Clod runs `git diff --stat HEAD` in each repo after reading docs. Tony fills context for anything significant.

**Out of scope:** `Groups/GUI/`, `Parse/BeforeRefactor/`, `Parse/Backup/` (new today), archive directories.

**Known current state:**
- Today's plg flatten committed: Parse/Revision/ → Parse/. 2 commits in plg repo (PLGrgx add + flatten itself) plus today's broader work.
- plgDirectives kept in Parse/ as live debugging tool, untracked-by-design
- documentation.md remains untracked WIP (separate conversation pending)
- Phase Triage FormatC.twk still uncommitted in tawk (waits on Phase Integrate)
- POP partial state: incant loads, first testCodE parses through aCTionCodE successfully, then double-define bug fires. Tony's overnight target.
- Bible amendments file in Tony's Downloads — reviewed, approved with glossing caveat

---

## 🔥 Immediate (current sprint)

### plg directory flatten — COMPLETE today

*Parse/Revision/* contents promoted to Parse/. Legacy plg sources moved to Parse/Backup/. .git relocated. plgDirectives kept live in Parse/. Splitter symlinks deleted. PLGrgx tracked into plg repo (was previously untracked source-of-truth in legacy Parse/).*

- [x] Pre-flight inventory
- [x] PLGrgx moved into plg repo (commit 139064b)
- [x] Parse/Backup/ created, dead legacy contents moved (plg.*, PLGitem, PLGlabel, PLGparse, PLGrule, PLGtester)
- [x] Splitter symlinks deleted (saved .twk copy in Parse/BeforeRefactor/)
- [x] Parse/Revision/* contents moved up to Parse/
- [x] .git relocated to Parse/.git
- [x] .gitignore audit: Backup/, BeforeRefactor/, Tests/, .claude/ all ignored
- [x] project.yml path reference for plgDirectives updated
- [x] plg.xcodeproj/project.pbxproj plgDirectives path updated
- [x] Doc reference sweep attempted, reverted (broke prose in TODO planning sections and bible directory maps; deferred to TODO refresh + bible refresh which rewrite those sections wholesale)
- [x] Commit ae06990 (flatten + drift cleanup)
- [x] Tony build verify: core build clean, navigator shows some stale file references (PLGrevision, plgDirectives, Include tok externals) — cosmetic, deferred to yaml-refresh activity

### CodE/DatA parseAction approach — committed yesterday

*Yesterday's grammar-level solution for code-block field values. testCodE = { maximus = 2; }; now parses through aCTionCodE atomically, bypassing checkSkip's indent-state issues. Implementation committed in a15471c.*

- [x] All implementation
- [x] Commit
- [ ] **Tony's after-hours target:** double-define bug. testCodE defines correctly once, then parser rewinds and defines it again. Synthetic-`:` insertion in checkSkip indent-mode is the suspected source.

### PLGset/CharSet — sister classes in support/Frame, committed yesterday

*PLGset moved from legacy Parse/ (untracked) to support/Frame (tracked, sister to CharSet). Both use inSet[256] representation. Resolved months of source-of-truth ambiguity.*

- [x] All work committed yesterday (commit 8223af6)

### Phase Integrate — Tokf migration to new plg (active arc)

*The big arc. Today's incant runtime works enough to make Phase Integrate the realistic next target. Migrate every Tokf source that uses old Buffer/PLGitem/PLGset/PLGtester surfaces to new ones. Build ~/bin/tokTemp. Validate Phase Triage. Eventually promote.*

- [ ] Recon: survey Tokf/ for files referencing old surfaces. Categorize migration shape.
- [ ] Sequence: pick a leaf-like file first.
- [ ] Migrate file by file, commit each.
- [ ] Reach clean compile.
- [ ] Build ~/bin/tokTemp.
- [ ] Smoke test (Phase Sandbox).
- [ ] Phase Triage runtime validation.
- [ ] Phase Promotion: ~/bin/tok ← tokTemp.

### Phase Triage — promoted to live source, awaiting Phase Integrate

*FormatC.twk lives in Tokf/. Uncommitted. Runtime validation waits on Phase Integrate producing a working binary.*

- [x] Design, staging, promote
- [ ] Commit (deferred)
- [ ] Runtime-validate via tokTemp

### Bible refresh — deferred to next session

*Bible amendments file in Tony's Downloads, reviewed and approved with glossing caveat. Full bible draft incorporates: today's flatten (directory map rewrite), the amendments from earlier in week, today's PLGset-in-support resolution. Mirror across four repos when done.*

- [ ] Clay drafts full revised bible (with today's flatten + all amendments)
- [ ] Tony reviews full bible draft
- [ ] Clod mirrors across four repos

### TAWK — Back Up and Running

#### Phase Splice ✅ COMPLETE (commit ef2730d, 2026-05-09)

#### Phase Triage — promoted to live source (see Immediate)

#### Phase Integrate — see Immediate (replaces Phase Port + Phase Compile)

#### Phase Lazarus — PLGsetParse revival (STANDBY)

#### Phase Loop — Self-host POP

---

### PLG — Self-hosting

- [ ] Action blocks feature
- [ ] Grammar reorganization

### Incant — testByteCode POP

- [ ] Add `Bytecode.mm` to incantGUI Xcode target
- [ ] Emitter rewrite: `gIF`, `gExpressioN`
- [ ] Verify `gBlocK`, `gFOR`, `gWhilE`, `gDO`
- [ ] Run testByteCode end-to-end

---

## 📋 Next Up

### PLG

- [ ] Wash & rinse cycle
- [ ] Support/Frame audit
- [ ] Xcode workspace (Shape B)
- [ ] **plg xcode link cleanup + yaml refresh** (post-flatten cosmetic work; Tony manually cleans navigator, then yaml-regen)

### TAWK

- [ ] TAWK autopsy remainder (after Phase Integrate)

### TOK Xcode project — yaml it (+ rename Groups → incant)

*Lives outside all four GitHub repos. No project.yml. Reverse-engineering from existing .pbxproj is the work. May also include renaming target.*

### Cluster C — Buffer 3-arg + new idiom adoption ✅ effectively complete

### Cluster D — Bytecode gating hook (PARKED)

### Cluster E — DEFINing flag / indent-as-structure (IN PROGRESS)

*Code-block-as-defining-region addressed via CodE/DatA parseAction (yesterday). Dedent-half of defining branch still open. Double-define bug from today's POP also lives here.*

### Incant

- [ ] `gPrinT`, `gXpress`, `gDeclare`
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project
- [ ] Distributed GroupItem messaging design

### Incant documentation conversation

*Tony's WIP on documentation.md surfaces in upcoming session. Untracked in plg repo working tree. Conversation-worthy.*

### GUI exploration recon (DEFERRED)

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
- [ ] PLGset/CharSet architectural note (stashed bible amendment — should now reflect today's support/Frame placement)
- [ ] Move Groups/GUI/ to a Reference/ sibling directory
- [ ] Move Groups/Maps/ to support source
- [ ] Accumulated working-tree drift sort: GroupDraw (parked, 76 lines), GroupControl (2), GroupItem (3), Stylish (2), KeyTable May 8 bulk-touch
- [ ] **Xcode-update discipline:** Clean Build Folder before debugging weird runtime behavior after Xcode update.
- [ ] **Visibility-gap discipline:** source-of-truth files MUST live in tracked locations. Today's PLGrgx (and yesterday's PLGset) resolutions exemplify the fix.
- [ ] **Tests/ just-in-case stash** — Parse/Tests/ contents are mostly dangling symlinks post-flatten. Tony may want a copy stashed somewhere just-in-case before fully forgetting about it.

---

## ✅ Done

### Recent (2026-05)

- [x] **plg directory flatten (2026-05-14)** — Parse/Revision/* promoted to Parse/, legacy material moved to Parse/Backup/, .git relocated, plgDirectives kept in place. PLGrgx tracked into plg repo as pre-flatten step (139064b). Flatten commit ae06990. Build clean for core code; navigator shows some stale file references for cosmetic cleanup later.
- [x] **CodE/DatA parseAction approach (2026-05-14, commit a15471c)** — grammar change to handle `{ ... }` field values atomically.
- [x] **PLGset migrated to support/Frame (2026-05-14, commit 8223af6)** — resolved months of source-of-truth confusion.
- [x] **CharSet rewrite committed (2026-05-14)** — landed with PLGset migration.
- [x] **Buffer migration to constructors (2026-05-13)** — bufferFactory{1,2,3,4} → three real C++ constructors.
- [x] **Phase Triage promoted to live source (2026-05-13)** — Tokf/Tests/FormatC.twk → Tokf/FormatC.twk.
- [x] **Include/changes restore (commit 17982d2, 2026-05-13)**
- [x] **TODO mirror push (2026-05-13)**
- [x] **PLGset / CharSet representation rewrite (2026-05-12)**
- [x] **Phase Triage staged & approved (2026-05-12)**
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
