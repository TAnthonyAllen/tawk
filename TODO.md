# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up

**Current state (end of 2026-05-16):**
- Incant unit-test suite passes clean. Overnight victory closed the precondition for Phase Integrate execution and Phase Bytecode Clod work.
- Phase Integrate Tonto recon 1, 2, and 3 all complete. Recon 3 produced the comprehensive migration scope: 5 files needing migration against current PLGitem interface, identified via direct comparison to PLGitem source.
- Phase Integrate migration 1 done: `.string()/.unString() → .toString()` mechanical pass across SymbolType.twk, Types.twk, Tawk.twk. Style upgrade, not compile-required — recon 3 revealed both methods still exist on current PLGitem.
- Phase Integrate migration 2 done: PLGitem invalid-surface migration (`iTEM[s] → iTEM.children[s]` and `iTEM.get(s) → iTEM.children[s]`) across the 4 small files: Symbol.twk (1 site), Directive.twk (2 sites), Instance.twk (1 site), SymbolType.twk (8 sites). 12 sites total, mechanical.
- **Tawk.twk's ~587-site invalid-surface arc remains.** 5 surface types: `iTEM[..]` indexing (264), `.testParser` (176), `.get(..)` (97), `.find(..)` (27), `.run()` (23). Asymmetric work — patterns vary per surface, design discussion likely needed before mechanical migration.
- BeforeRefactor/ is the FileMerge baseline. Frozen by design, not maintained as current. FormatC.twk and Tawk.twk show stale snapshots and that's *correct* — reflects prior in-flight work as the diff baseline.
- TOK xcode reconfig (point at Tests/-derived sources) is Tony's seat. Pending; waits on either completion of Tawk.twk migration or Tony's call that it's time to test-build against the 4-small-files migration.

**First work options after wake-up read:**

- **Tawk.twk migration arc.** ~587 sites across 5 surface types. Likely starts with a recon-shape brief enumerating per-surface migration patterns (similar to recon 2 for `.string()/.unString()`), then mechanical migration in passes. The scale and asymmetry warrant a design conversation before committing Clod to mechanical work.

- **TOK xcode reconfig + build attempt.** Tony's seat. Validates the build path (Tests/ → tok → .C → Xcode compile) against the 4-small-files migration before committing to Tawk.twk. Cheap to try; reveals whatever's broken in the path before scope grows. Could happen before *or* after Tawk.twk arc — Tony's call.

- **Phase Bytecode start.** Unit-test gate cleared, Phase Integrate not blocking. Filling in gIF and gExpressioN as incant generators producing bytecodE attributes. Twin POP: testByteCode runs end-to-end with `maximus = 26` AND generator dispatch demonstrated for the bytecode case.

- **CLAUDE.md drift fix.** Promoted to Immediate. Hasn't happened yet. Could be done before any of the above; mostly mechanical alignment with bible v2.

- **HWF graduation ritual for Sessions 4 and 5.** First real test of the ritual. Material settled; needs proper trim drafting.

- **Cha cha session work** — Session 1 (isCLAUDE plus wake-up scripts thread plus the operational patterns accumulating). Whenever appetite supports it.

**Reading targets:**
- `Parse/` — plg repo root. All plg source lives here directly.
- `Parse/Backup/` — parked legacy plg material (gitignored)
- `Parse/PLGitem.twk` — source of truth for the PLGitem interface. Recon 3 referenced this directly.
- `Parse/HWFattic/` — graduated HWF session trims. Empty as of 2026-05-16; Session 4 and 5 graduation pending.
- `support/Frame/PLGset.{twk,C,h}` and `support/Frame/CharSet.{twk,C,h}` — sister classes, source of truth
- `Tokf/` — TAWK source. Originals; not edited directly during Phase Integrate.
- **`Tokf/Tests/`** — where Phase Integrate migration edits actually land. Symlinks-back-to-Tokf/ by default; replaced with real-file copies on a per-file basis as migration touches each file. As of end-of-2026-05-16, real-file copies exist for: FormatC.twk (predates), SymbolType.twk, Types.twk, Tawk.twk (migration 1), Symbol.twk, Directive.twk, Instance.twk (migration 2). Remaining files are still symlinks.
- `Tokf/BeforeRefactor/` — FileMerge baseline. **Frozen by design, do not update.** Stale entries are expected and correct.

**Standing wake-up practice:**
Clod runs `git diff --stat HEAD` in each repo after reading docs. Tony fills context for anything significant.

**Out of scope for current Phase Integrate arc:** `Parse/BeforeRefactor/`, `Tokf/BeforeRefactor/`, archive directories, `Groups/GUI/` (GUI work deferred; incant CLAUDE.md still covers its general role).

**Known current state:**
- Bible v2 mirrored across all four repos (2026-05-15). Phase naming: Phase Generate Tawk, Phase Integrate, Phase Bytecode, Phase JIT.
- jit.md mirrored across all four repos as sibling to bible (2026-05-15).
- Incant POP fully working as of 2026-05-16: runs to completion, test action fires end-to-end, full unit-test suite passes.
- Phase Triage FormatC.twk still uncommitted in tawk (waits on Phase Integrate to produce a working binary).
- Tests/ working-tree state (the 7 real-file copies) is the deliverable for the migration work to date. Tests/ is gitignored; no commits.

---

## 🔥 Immediate (current sprint)

### Phase Integrate — Tokf migration to new plg (ACTIVE)

*The big arc. Incant unit-test suite passing as of 2026-05-16 cleared the precondition. Recon 1, 2, 3 done. Migration 1 and 2 done. Tawk.twk's 587-site invalid-surface arc remains.*

*Strategy: clear non-plg-bound .twk files first (done for 4 small files; Tawk.twk remains the mega-cluster). Then .g/.act pairs — the actual plg integration tar — get their own coordinated arc later.*

*Tony's framing: once Tawk + new plg compiles (even buggy), Tony's Xcode debugger comes online and Tony chips in directly, same shape as overnight unit-test work. Goal is compile, not cleanliness. Bugs after compile are features.*

- [x] Recon 1: surface count and categorization across Tokf/
- [x] Recon 2: per-file migration shape for the 3-file `.string()/.unString()` surface
- [x] Recon 3: comprehensive migration scope against current PLGitem interface
- [x] Migration 1: `.string()/.unString() → .toString()` across SymbolType.twk, Types.twk, Tawk.twk (~81 sites). Style upgrade.
- [x] Migration 2: PLGitem invalid-surface migration across 4 small files (Symbol, Directive, Instance, SymbolType — 12 sites)
- [ ] Tawk.twk invalid-surface migration: ~587 sites across 5 surface types. Likely needs a recon-shape brief before mechanical migration. **Next major work item.**
- [ ] TOK xcode reconfig to point at Tests/-derived sources (Tony's seat)
- [ ] Build attempt against the 4-small-files migration (validates build path before Tawk.twk arc commits)
- [ ] Migration: .g/.act pairs (separate coordinated arc, scoped later)
- [ ] Reach clean compile against new plg
- [ ] Build ~/bin/tokTemp
- [ ] Tony Xcode debug work on integrated build
- [ ] Smoke test (Phase Sandbox)
- [ ] Phase Triage runtime validation
- [ ] Phase Promotion: ~/bin/tok ← tokTemp

### Phase Bytecode — incant bytecode emitter and interpreter (UNBLOCKED 2026-05-16)

*Unit-test precondition cleared. Plan: fill in gIF and gExpressioN as incant generators producing bytecodE attributes. Incant-first emission per Tony's design preference. C++ emitter fallback only on demonstrated infeasibility.*

*Twin POP: testByteCode runs end-to-end with `maximus = 26` AND generator dispatch demonstrated for the bytecode case.*

- [ ] Bytecode.mm → Xcode target (manual: drag into incantGUI)
- [ ] Fill in gIF in Generate.rtn — produce bytecodE attributes
- [ ] Fill in gExpressioN in Generate.rtn — produce bytecodE attributes
- [ ] Verify gBlocK, gFOR, gWhilE, gDO interact correctly with new gIF/gExpressioN output
- [ ] Run testByteCode end-to-end
- [ ] Capture bytecode emission shape in jit.md once settled

### CLAUDE.md drift fix (PROMOTED from Housekeeping 2026-05-16)

*Bible v2's resurrection-reader standard applies to all .md files. Incant CLAUDE.md still has pre-flatten Parse/Revision/ paths and old "Phase 2" framing for bytecode work. Primary-standard violation, not housekeeping.*

*Scope: all four repos' CLAUDE.md files. Bring each into agreement with bible v2's directory map, phase naming, and current state.*

- [ ] Audit incant CLAUDE.md against bible v2
- [ ] Audit plg CLAUDE.md against bible v2
- [ ] Audit support CLAUDE.md against bible v2
- [ ] Audit tawk CLAUDE.md against bible v2
- [ ] Add `InProcess/InProcess.xcworkspace` to all CLAUDE.md files (was on Housekeeping)
- [ ] Mirror updates across four repos

### HWF.md graduation ritual — Sessions 4 and 5 to attic (2026-05-16)

*First real test of the graduation ritual. Session 4 (indentation as structure) and Session 5 (PLGset/CharSet split) both substantially settled. Decisions in bible. Definitions earned. Open questions resolved or transferred.*

*HWF.md location: verify before drafting. Currently believed to be Parse/HWF.md only (single-source in plg, not mirrored). If HWF.md is single-source, "mirror across four repos" doesn't apply and the graduation work lands in one place. Confirm with Tony at session start.*

- [ ] Verify HWF.md location (single-source in plg, or mirrored across all four)
- [ ] Verify graduation conditions for Session 4 (all decisions in bible, open questions resolved or transferred)
- [ ] Verify graduation conditions for Session 5 (placement landed, captured in bible Architecture section)
- [ ] Check `Parse/HWFattic/` — empty currently, no older sessions waiting
- [ ] Create `session4indentation.md` in HWFattic with Session 4's final trim
- [ ] Create `session5plgsetcharset.md` in HWFattic with Session 5's final trim
- [ ] Remove Session 4 and 5 from HWF.md active sessions
- [ ] Update HWF.md Sessions index — Active section, Graduated section
- [ ] Mirror HWF.md update if it's mirrored; otherwise skip

---

## 📋 Next Up

### Bible refresh — minor sync passes (after major arcs settle)

*The bible v2 from 2026-05-15 is substantially current. Small drift items accumulate:*

- [ ] Session 6 (parse error handling) — add to bible's HWF Sessions Pending Work index when refresh happens
- [ ] Session 9 status — Session 9 (wake-up scripts) was originally queued as a separate session; per 2026-05-16 cha cha discussion, folded into Session 1 as a sub-thread rather than separate session. Bible's HWF index needs to reflect this (no Session 9; Session 1 expanded to cover wake-up scripts thread).
- [ ] PLG self-host status — currently hedged "unknown until next attempt." A future Tonto run could confirm cheaply. Worth doing during a low-stakes Tonto window.
- [ ] PLG Next items status pass — happens when Phase Integrate brings us back deep into plg work

### PLG — Self-hosting

- [ ] Action blocks feature
- [ ] Grammar reorganization
- [ ] **Paren-alt decomposition for incant** — port BlockplgAct from PLG. Reference design is the PLG implementation. Low priority.

### Phase Integrate — extended

- [ ] TAWK autopsy remainder (after Phase Integrate completes)
- [ ] Scoped TAWK autopsy (independent): GC inheritance fix, include guard fix — go into legacy Tokf/Tawk.twk directly

### TOK Xcode project — yaml it (+ rename Groups → incant)

*Lives outside all four GitHub repos. No project.yml. Reverse-engineering from existing .pbxproj is the work. May also include renaming target. Housekeeping for whenever.*

### plg xcode link cleanup + yaml refresh

*Post-flatten cosmetic work. Tony manually cleans navigator, then yaml-regen. Build is fine without it.*

### Incant — beyond Phase Bytecode

- [ ] `gPrinT`, `gXpress`, `gDeclare` — fill in remaining stubs once Phase Bytecode shape settles
- [ ] `genPrint` in Generate.rtn — replace with bytecode equivalent
- [ ] `runCall` handler
- [ ] JSON rule — find in attic, POP
- [ ] Bot messaging project
- [ ] Distributed GroupItem messaging design

### Incant documentation conversation

*Tony's WIP on documentation.md surfaces in upcoming session. Untracked in plg repo working tree. Conversation-worthy. Tony "needs a wee bit more time to get ready for it" — postponed 2026-05-16.*

### Cluster D — Bytecode gating hook (LANDED, hook in GroupRules.mm:786)

### Cluster E — DEFINing flag / indent-as-structure ✅ EFFECTIVELY COMPLETE

*Both halves resolved: CodE/DatA atomic parseAction (2026-05-14) and checkSkip double-define fix (2026-05-15). Full unit-test pass (2026-05-16) confirms no regressions.*

### GUI exploration recon (DEFERRED)

### Maps → move to support source

### TOK build machinery lives outside all four GitHub repos

---

## 🔭 Longer Term (HPDL)

- [ ] Claude as native GroupItem field type (`isCLAUDE`) — Session 1 design work pending
- [ ] Incant as distributed virtual OS
- [ ] Go-style channel messaging
- [ ] ZFS-flavored storage
- [ ] Incant display/layout field
- [ ] File system as GroupItems
- [ ] PLG written in Incant
- [ ] Incant self-hosting via JIT — Phase JIT, design pending Session 8
- [ ] Xcode-like development environment written in incant

---

## 🗂️ Housekeeping

- [ ] plg.g `%%` assumption — document/fix
- [ ] doNotGuard accumulation
- [ ] +1000 offset reporting quirk
- [ ] ~/bin/plg dated Nov 2024 — verify or rebuild
- [ ] Support repo update process — needs a look
- [ ] Move Groups/GUI/ to a Reference/ sibling directory
- [ ] Move Groups/Maps/ to support source
- [ ] Accumulated working-tree drift sort: GroupDraw (parked, 76 lines), GroupControl (2), GroupItem (3), Stylish (2), KeyTable May 8 bulk-touch
- [ ] **Xcode-update discipline:** Clean Build Folder before debugging weird runtime behavior after Xcode update.
- [ ] **Visibility-gap discipline:** source-of-truth files MUST live in tracked locations. PLGrgx and PLGset resolutions exemplify the fix.
- [ ] **Tests/ just-in-case stash** — Parse/Tests/ contents are mostly dangling symlinks post-flatten. Tony may want a copy stashed somewhere just-in-case before fully forgetting about it.
- [ ] **PLGset.init() stub** — dead code, retained for API compatibility with older lazy-parse lineage. Can be removed in support/Frame cleanup pass.

---

## ✅ Done

### Recent (2026-05)

- [x] **Phase Integrate migration 2 (2026-05-16)** — PLGitem invalid-surface migration (`iTEM[s] → iTEM.children[s]` and `iTEM.get(s) → iTEM.children[s]`) across 4 small files in Tokf/Tests/. 12 sites total: Symbol.twk (1), Directive.twk (2), Instance.twk (1), SymbolType.twk (8). All sites clean, receiver-type sanity check passed across all 12.
- [x] **Phase Integrate Tonto recon 3 (2026-05-16)** — comprehensive migration scope against current PLGitem interface. 5 files need migration: Symbol, Directive, Instance, SymbolType (the 4 small files migrated in migration 2), plus Tawk.twk (587 invalid-surface sites across 5 types, separate arc). Surfaced that `.string()/.unString()` are still valid on current PLGitem — migration 1 was a style upgrade, not a compile-required fix. BeforeRefactor/ verified: 11 of 13 files current, 2 expected-stale.
- [x] **Phase Integrate migration 1 (2026-05-16)** — `.string()/.unString() → .toString()` style migration in Tokf/Tests/ across SymbolType.twk (1 site), Types.twk (1 site), Tawk.twk (79 sites). Symlinks replaced with real copies. Tests/ stays gitignored — working-tree state is the deliverable.
- [x] **Phase Integrate Tonto recon 2 (2026-05-16)** — per-file migration shape for `.string()/.unString()` surface. Surfaced that PLGset API in Types.twk was misclassified as legacy by recon 1 (Clay-side Category-4 triage failure — filed as cha cha pattern).
- [x] **Incant unit-test suite passing (2026-05-16)** — overnight victory. Closed precondition for Phase Integrate execution and Phase Bytecode Clod work. POP confirms the May 15 checkSkip fix didn't regress anything else.
- [x] **Phase Integrate Tonto recon 1 (2026-05-16)** — surface count and categorization across 22 active Tokf/ files. Strategy locked: clear non-plg-bound .twk first, concentrate tar in .g/.act pairs.
- [x] **Bible v2 + jit.md mirrored across four repos (2026-05-15)** — Phase naming convention extended (Phase Generate Tawk, Phase Integrate, Phase Bytecode, Phase JIT), bare-include framing retired, HWFattic and Generators glossary entries added, Incant Core Concept paragraph added, GroupItem prose line added, HWF Sessions 6 and 8 queued.
- [x] **checkSkip double-define bug fixed (2026-05-15)** — testCodE no longer rewinds after aCTionCodE. The `;;` runtogether and `:`/`>` non-user-facing rules earned as residual user-facing constraints. checkSkip indent-mode hardened.
- [x] **PLGmain split from PLGparse (2026-05-15)** — class wrapper owns main(), PLGparse is library citizen only. Linking against PLGparse no longer drags PLGparse's old main() in.
- [x] **plg directory flatten (2026-05-14)** — Parse/Revision/ → Parse/, legacy material to Parse/Backup/, .git relocated. Flatten commit ae06990. PLGrgx tracked into plg repo (139064b).
- [x] **CodE/DatA parseAction approach (2026-05-14, commit a15471c)** — grammar change to handle `{ ... }` field values atomically, bypassing checkSkip indent-state issues.
- [x] **PLGset migrated to support/Frame (2026-05-14, commit 8223af6)** — resolved months of source-of-truth confusion. Sister to CharSet.
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
