# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up (2026-05-13)

**First work:** Multiple threads converged today. Pick one to lead with — they don't all need to happen tomorrow, but they all want attention:

- **Phase Triage promotion.** Tony reviewed Tokf/Tests/FormatC.twk via fileMerge today and approved the changes. Tomorrow: promote Tests/FormatC.twk to live Tokf/FormatC.twk, rebuild ~/bin/tok, run against a sample .twk to confirm output has include guards + `: public gc`.

- **Commit PLGset/CharSet rewrites.** Today's representation rewrite (bit-packed `map[4]` → `unsigned char *inSet`, 256 bytes, debuggable) is in working tree, untested beyond compile-and-run. Build is clean and the original morning corruption symptom is gone. Worth committing after a fresh-eyes look tomorrow.

- **checkSkip dedent for defining mode.** Today's session pinned the next bug: the dedent-half of the `defining` indent-check branch is commented out in checkSkip. When `:`-opened blocks dedent, no synthetic close fires, and the block doesn't close. Visible failure: `generator:` in the generate file absorbs `generatE` (which should be a peer in the outer define block). Per Clod's wake-up read yesterday morning: `//or defining { *atContent = ';'; }` at line ~195 of GroupRules.twk.

- **Code-block-as-defining-region.** Still open. Yesterday's `defineSkipSet` attempt broke setBrackets (the `{` in `[{A-Za-z0-9+-;(]` was treated as a brace block). Real design work needed for context-aware `{` handling — Tony's continued overnight noodle.

**Tony's overnight work also includes:**
- Reviewing PLGset/CharSet rewrites in working tree
- Continued noodle on code-block-as-defining-region design

**Reading targets for Clay and Clod:**
- `support/Frame/CharSet.twk` — rewritten today, uses `unsigned char *inSet` representation
- `Groups/PLGset.twk` — rewritten today, same representation
- `Tokf/Tests/FormatC.twk` — staged Phase Triage changes, ready for promotion
- `Groups/GroupRules.twk` — checkSkip() comment fix from 2026-05-11 is committed; dedent issue at line ~195 is next

**Out of scope (do not browse, survey, or modify):**
- `Groups/GUI/` — HPDL (Tony may unleash Clod-recon as a deliberate exception later)
- `Parse/BeforeRefactor/` — archive

**Known current state:**
- Working tree carries today's rewrites: PLGset.twk, CharSet.twk, and the corresponding .h/.mm regen artifacts. Plus the dirty state from prior sessions (FormatC sandbox, defineSkipSet in GroupRules, etc.). None committed today.
- Yesterday's `support/Include/frame` external for CharSet was updated to match the new representation (per Tony's update during today's session).
- Build still clean. PLGset and CharSet rewrites compile and run.
- Morning corruption symptom (ruleSkipSet.contains returning false on `\n`) appears to be resolved by the rewrite — the bit-packed `map[4]` representation was load-bearing in the bug somehow. Replaced with `inSet[256]` and the symptom is gone.

---

## 🔥 Immediate (current sprint)

### Phase Triage — vetted, ready for promotion tomorrow

*Tokf/Tests/FormatC.twk has three edits: GC inheritance in C++ class declarations, guardTokenFromName helper, and close() modified to emit include-guard prologue + `#endif`. Tony reviewed via fileMerge today and approved.*

- [x] Recon: identify edit sites in FormatC.twk
- [x] Design: three edits — GC inheritance, helper, close() modification
- [x] Apply edits to Tokf/Tests/FormatC.twk (sandbox)
- [x] Tony reviews via fileMerge (2026-05-12, approved)
- [ ] Promote Tests/FormatC.twk to live Tokf/FormatC.twk
- [ ] Rebuild ~/bin/tok from the updated FormatC
- [ ] Run sandboxed ~/bin/tok on a sample .twk file (one of the support classes?)
- [ ] Verify output: include guards in .h, `#include "gc/gc_cpp.h"`, `: public gc` inheritance for C++ classes
- [ ] If clean, commit and ship

### PLGset / CharSet — rewrite landed, needs commit

*Today's representation rewrite. Storage changed from bit-packed `unsigned long *map` (4 longs, MSB-first packing) to `unsigned char *inSet` (256 bytes, one per char codepoint). Trivially debuggable: `inSet[10]` is either 0 or 1, no decoding needed. Negation resolved at construction time (input `^abc` produces `inSet[]` with 253 entries set), `negated` flag retained as informational only. toText removed; toString rewritten to print decimal-value-and-char for each present char.*

*Mechanically: field declared as `unsigned char *inSet`, both constructors allocate via `calloc(256, sizeof(char))` as first line. Tok auto-init of `inSet = 0` works because the field is a pointer (not an array). Same fix applied to both PLGset and CharSet.*

*Result: today's morning corruption symptom (ruleSkipSet.contains returning false on `\n` when it should be true, accompanied by 95 chars appearing in toString output) is gone. The bit-packed representation's MSB-first bit math may have been correctness-fragile in some specific way that the new representation eliminates.*

- [x] Rewrite PLGset.twk with `unsigned char *inSet` storage
- [x] Rewrite CharSet.twk with parallel representation
- [x] Update `support/Include/frame` external for CharSet
- [x] Build both, run incant loader
- [ ] Tony reviews working tree state with fresh eyes
- [ ] Commit when satisfied
- [ ] Verify against more incant files than today's load test exercised

### checkSkip — dedent for defining mode

*Today's session pinned this. Without `:` on `generator`, it defines empty and members become outer-block peers. With `:`, it opens the member block correctly, but the block doesn't close — `generatE` (peer in outer block) gets absorbed as the 10th member of `generator`. Cause: the dedent-half of the `defining` branch in checkSkip's indent-check is commented out — `//or defining { *atContent = ';'; }` at line ~195 of GroupRules.twk.*

*Tonto noted this gap yesterday morning at wake-up (Surprise 2). Today it's the live blocker for indent-as-structure POP.*

- [ ] Design: what token should fire on `defining`-mode dedent? `;`? Something else?
- [ ] Apply the dedent fix to checkSkip
- [ ] Verify generator define block closes correctly before generatE
- [ ] Verify no regression on `:`-opened blocks that *should* keep growing

### checkSkip — code-block-as-defining-region (Tony's overnight, still open)

*Open since 2026-05-11. Inside `{ ... }` code blocks within a defining context, checkSkip processes the contents and mutates indent state, producing wrong results downstream. Yesterday's `defineSkipSet` approach broke `setBrackets` (the `{` inside `[{A-Za-z0-9+-;(]` was treated as a brace block when it's actually a character in a `[...]` set declaration). Needs context-aware design.*

- [ ] Tony's continued noodle: design context-aware `{` handling
- [ ] Apply, test against drawing file's Point + arc + curve definitions
- [ ] Verify no regression in setBrackets or anywhere else `{` appears as content

### Xcode environment — workable dev loop ✅ DONE (2026-05-11)

### Incant — PLGitem migration vet ✅ DONE in effect (2026-05-11)

### TAWK — Back Up and Running

*Get TAWK incorporating the revised PLG. Floor POP: new TAWK processes the .twk files in plg/incant/support/tawk. Stretch POP: `plg Tawk.g` → Tawk.twk → new TAWK builds → new TAWK re-tokkifies the ecosystem. Self-consistent loop.*

#### Phase Splice ✅ COMPLETE (commit ef2730d, 2026-05-09)

#### Phase Triage — staged & vetted, ready for promotion (see Immediate above)

#### Phase Port — Callback signature migration

*~50+ `*TawkNow` and `*TawkAct` callbacks change signature to `(PLGparse *state, PLGitem *iTEM)`. Largest single piece of work in the arc.*

- [ ] Inventory the callbacks — count, group by mechanical-vs-design
- [ ] Decide session granularity (Tony + Clay)
- [ ] Port callbacks (one or more Clod sessions)
- [ ] If callbacks reference PLGsetParse → activate Phase Lazarus

#### Phase Compile — First clean build

- [ ] Build new TAWK against current support + revised PLG
- [ ] Resolve unresolved references
- [ ] If link errors point at PLGsetParse → activate Phase Lazarus
- [ ] Clean compile achieved

#### Phase Lazarus — PLGsetParse revival (STANDBY)

#### Phase Sandbox — Tests/ verification vs ~/bin/tok

#### Phase Promotion — Replace ~/bin/tok

#### Phase Loop — Self-host POP

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

- [ ] TAWK autopsy remainder (after Phase Triage)

### TOK Xcode project — yaml it (+ rename Groups → incant)

### Cluster C — Buffer 3-arg + new idiom adoption

### Cluster D — Bytecode gating hook (PARKED)

### Cluster E — DEFINing flag / indent-as-structure (IN PROGRESS)

*Comment handling fixed 2026-05-11. Code-block handling open. Dedent-half of defining branch needs implementation. Today's session sharply defined both remaining pieces.*

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

*Maps directory (containing Segment.twk with bitMask and friends) currently lives in Groups but should be in support. Move when convenient. Doesn't matter for the inSet[256] rewrite since neither PLGset nor CharSet uses bitMask anymore — but Maps still has callers elsewhere presumably.*

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
- [ ] Bible safety-rule amendment (line 209) — clarify Phase 2 arc as explicit exception
- [ ] Support repo update process — needs a look (drift between local and GitHub)
- [ ] Bible update — TAWK Known Issues autopsy table item 12: post-increment-dereference on RHS of type-aware `+=` (parse.rtn:94 bisect 2026-05-10)
- [ ] Bible update — add "Clay-designs-Clod-applies-Tony-reviews-before-tok-time" as named cha cha mode (2026-05-11)
- [ ] Add "out-of-scope directories" standing rule to bible or CLAUDE.md
- [ ] Move Groups/GUI/ to a Reference/ or Stealable/ sibling directory
- [ ] Move Groups/Maps/ to support source
- [ ] `git add <file> && git commit` discipline: confirm index contents before commit

---

## ✅ Done

### Recent (2026-05)

- [x] **PLGset / CharSet rewrite (2026-05-12)** — replaced bit-packed `map[4]` representation with `unsigned char *inSet` (256-byte storage, one per codepoint). Trivially debuggable. Negation resolved at construction time. toText removed; toString rewritten with decimal-value output. Both compile clean. Morning ruleSkipSet corruption symptom is gone.
- [x] **Phase Triage staged in Tokf/Tests/FormatC.twk (2026-05-11)** — three edits: GC inheritance, guardTokenFromName helper, close() emits include-guard prologue + `#endif`. Tony reviewed and approved 2026-05-12. Pending promotion.
- [x] **checkSkip comment-in-define-block fix (commit a219689, 2026-05-11)** — line 159 over-advancement past `*/` close fixed; comment processing now correctly leaves cursor on the following newline so the newline handler can reset indent state.
- [x] Xcode dev loop working (2026-05-11) — Tony got the build clean overnight.
- [x] PLGset.addTest() removed (2026-05-11) — was never called. Stashed in Parse/BeforeRefactor/.
- [x] RuleStuff fix (commit 0835c34, 2026-05-10) — parse.rtn:94 split-form workaround for tok post-increment-on-+= bug.
- [x] Cluster B regen + revert (commit 6398920, 2026-05-10) — regenerated four .mm/.h pairs from .twk source-of-truth.
- [x] Move five working files (commit fec9358, 2026-05-10).
- [x] Buffer source-of-truth verified (2026-05-10).
- [x] Phase Splice complete (commit ef2730d, 2026-05-09).
- [x] PLG `process()` CWD-relative path contract (commit da51193).
- [x] Bible May 7 polishes.
- [x] Bible mirror sweep across all four repos.
- [x] Incant repo cleanup commit (b5375e8).
- [x] HWF.md trim ritual added.
- [x] Verification protocol added.

### Earlier

- [x] Four repos created and public (plg, incant, support, tawk)
- [x] CLAUDE.md, TODO.md, projectBible.md present in all four repos
- [x] "What Is Incant?" wiki page
- [x] Support static library (libsupport.a)
- [x] PLGset moved to support repo
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
