# TODO.md — PLG/TAWK/Incant Ecosystem

*Read this first every session. Keep it current.*

---

## Tomorrow's wake-up

**Current state (end of 2026-05-24, Phase Bytecode arc mid-flight):**
- **Phase Bytecode Brief 3 verification advanced but not closed.** Today's session produced three concrete fixes and surfaced two specific remaining questions:
  - Three overnight bug fixes landed (2026-05-24 Tony work): runAction double-unwrap removed (runOP already unwraps; runAction was doing it again), printField zero-result handled (dot-op returning 0 was crashing printField; now substitutes falseResult global), bcLIST linkage fix (local bcLIST.group now points at the real bcLIST rather than receiving setContent of it).
  - First gating attempt at invokE during generation broke runGenerated dispatch — `runGenerated: action is: Token` instead of `gIF`/`gBlocK`. Root cause identified: lazy parsing of generator method bodies (gIF, gBlocK, gXpress) was happening *under* the gating flag, so their statement-Tokens didn't get invokE set during their first parse, breaking dispatch.
  - Refactor fix landed (2026-05-24 Tony work): `generating` flag moved from GroupRules global to GroupBody per-action; set in generateCode() against the action being generated; checked by TokenXP and ExpressioN actions via currentMETHOD. Dispatch regression cleared.
  - bcLIST linkage fix's effect is visible in testByteCode (six entries, pair pattern) but NOT in testEmitBC or testGXLeaf (still showing `single field xp`). Asymmetry across the three tests is the new finding.
- **Two open questions parked for Tony's after-hours / next-session investigation:**
  - **isLiteraL=1 on operator Tokens.** `>` token reads as isLiteraL=1 in gXpress, hitting the leaf-emit branch instead of operator-descent. Either legitimate ("isLiteraL means matched-as-literal-text," so `>` correctly carries it and gXpress needs a different operator-discriminator like `Operators[token.taG]`) or misclassification (flag should only be on value-literals; bug upstream of gXpress). Clod tasked 2026-05-24 with locating where isLiteraL is read in the generate-action layer to give Tony location data for debugger investigation.
  - **bcLIST scope-resolution asymmetry.** emitBC's `:generator bcLIST` reaches the right bcLIST when emitBC fires inside a for-loop body (testByteCode case) but not when it fires outside one (testEmitBC, testGXLeaf cases). Also "two entries per emit" inside the for-loop case is a separate sub-puzzle. Tony to investigate.
- **Cha-cha practice update 2026-05-24:** Finding-location reporting refined. When generator-side or parse-action-side findings surface, Clod's report names file path, action/method name, line number, and conditions. C++ layer archaeology (GroupRules.mm, Parser, Tokenizer) stays in Tony's lane unless explicitly delegated. Reason: Tony needs location data to debug efficiently; broad symptom descriptions force re-investigation. For Clay: when summarizing findings for Tony, flag location-unknown items explicitly rather than handing them over as if settled.
- **Lucky-coincidence trap caught us three times this week.** testGXLeaf "passing" was leaf-fallthrough on single-Token ExpressioN (May 22), "works as intended" overnight (May 23) didn't survive dumpBC walk on Brief 3 verification (May 24 morning), and bcLIST `single field xp` symptom on May 23 was masked by emit lines visible in trace. Pattern of "appears to work" diverging from "bones-confirmed" earns its place in bible Working Relationship section at next refresh.
- **Wiki first-pass restructure landed 2026-05-24.** What-Is-Incant page revised: bootstrap section moved to Appendix B, For the Nerds moved to Appendix A, JSON/YAML progression moved earlier in flow. Tone preserved. Weekly cadence proposed for ongoing wiki refinement. Features+working-examples list as separate destination (rather than single illustrative program).
- **Tawk recon graduated 2026-05-20** as durable reference at `Tokf/docs/fieldResolutionRecon.md`. Six findings tracked in TODO. First Tonto-shaped commander-discretion archaeological recon in the cha cha — pattern worked.
- **docs/ convention landed 2026-05-19** across all four repos. Working-level plans and design docs live in `docs/`. Graduated session trims live in `HWFattic/`.
- **Three cha-cha modes now demonstrated in practice**: woodshed (Clay+Tony plan-shape), brief-execute (Clod mechanical-shape), commander-discretion (Tonto archaeological-shape).

**First work options after wake-up read:**

- **Brief 3 verification finish line** — two open questions parked from yesterday's session, both Tony-seat: (a) isLiteraL=1 on operator Tokens — Clod's location-report from 2026-05-24 should make this debuggable from Tony's side; once root cause is settled, either fix isLiteraL upstream or change gXpress discriminator to Operators[token.taG]; (b) bcLIST scope-resolution asymmetry across the three tests — investigate why `:generator bcLIST` reaches the right bcLIST inside for-loop body but not outside. Both questions cleanly characterized; not stuck. Once both settle, re-run all three tests for clean Brief 3 close. Then Brief 4 (gIF branch+marker) becomes the natural next move.
- **Print bytecode plan document** — `Groups/docs/printGenerationPlan.md`. Substantial design work surfaced during 2026-05-21 woodshed: print machinery covers PrinT and StringXP via aCTionPrinT; bytecode design has bcPrintShortcut/bcPrintField/bcPrintEmit/bcStringEmit; `=:` operator parked as future grammar change to collapse `string` keyword. Worth a session to write up as durable artifact.
- **`=:` operator grammar-change design** — assignment operator with string-build semantics on RHS, replaces `string` keyword usage. Open question: Path A (operator in ExpressioN with sub-grammar switch) vs Path B (separate statement form). plg conditions may be the parser-switch mechanism. Parallel design, not blocking gPrint.
- **Wiki weekly refinement** — first-pass restructure landed 2026-05-24. Weekly cadence proposed. Next pass candidates: features-and-working-examples list (incant-by-Clod-discovery list from yesterday is the first-cut source — but only language *features*, not collaborator-level idioms); polish notes from first pass (CodE bootstrap rule paste artifact, methods/actions reconciliation, field/GroupItem terminology consistency).
- **Tawk.twk migration arc** — 587 sites across 5 surface types. Now informed by Tonto's recon. Could start with a follow-on recon-shape brief enumerating per-surface migration patterns.
- **Session 9 follow-up items** (small, can interleave): Stak[] operator, PLGmain.twk modification, hard-coded include paths, plg modifier coverage audit vs incant, etc.
- **HWF graduation ritual for Sessions 4 and 5** — still pending. Sessions 9 and Phase Bytecode (when it closes) both prove the ritual works.

**Reading targets:**
- `Groups/` — incant repo root. Phase Bytecode work lives here.
- `Groups/XML/WorkingOn/generate` — incant generator actions including gXpress, gIF, emitBC. Active file.
- `Groups/XML/WorkingOn/oneTest` — test harness with testByteCode invocation.
- `Groups/ruleActions.rtn` — aCTionExpressioN, aCTionTokenXP, opDot machinery. Touched during the 2026-05-21→22 incant-machinery investigations (now resolved).
- `Groups/Instruct.rtn` — opDot machinery. Unwrap change resolved 2026-05-22.
- `Tokf/docs/fieldResolutionRecon.md` — Tawk field resolution recon, durable reference.
- `Parse/HWFattic/session9plgDebugAndActions.md` — graduated Session 9 trim.
- `Parse/docs/Session9plan.md` — Session 9 working-level plan.
- `Tokf/` — TAWK source. Originals; not edited directly during Phase Integrate.
- **`Tokf/Tests/`** — where Phase Integrate migration edits land when that arc resumes.

**Standing wake-up practice:**
Clod runs `git diff --stat HEAD` in each repo after reading docs. Tony fills context for anything significant.

**Standing uploads for Clay (added 2026-05-23):**
Clay (claude.ai) has no filesystem access — works from `.md` files and any code Tony uploads. Repo fetches from GitHub lag working-tree state, sometimes badly. Tony's standing practice: upload working-tree of active-surface files at session start. For Phase Bytecode, that's currently: `Groups/XML/WorkingOn/generate`, `Groups/Generate.rtn`, `Groups/XML/WorkingOn/grammar` (when grammar shape comes up), `Groups/XML/WorkingOn/setup`, `Groups/XML/WorkingOn/unitTests`, `Groups/XML/WorkingOn/oneTest`, `Groups/Bytecode.mm`, `Groups/Bytecode.h`. List shifts when arc shifts. Pattern: review repos as background orientation; uploads provide current-state ground truth. Clay's discipline: when reasoning about specific source content, ask for current version rather than reason from memory or GitHub fetch.

**Finding-location standing practice (added 2026-05-24):**
When a generator-side or parse-action-side finding surfaces, Clod's report names:
- File path
- Action/method name
- Line number or distinctive context
- Conditions under which the behavior fires

Within the incant/twk/tok layer. C++ archaeology (GroupRules.mm, Parser, Tokenizer) is Tony's seat — Clod doesn't dive in unless explicitly asked. Reason: Tony needs location info to debug efficiently; "the surface fires somewhere in gXpress" is too broad; "gXpress line 285 in the `or token.isLiteraL;` branch" lets Tony go directly to source. When source-reading doesn't reveal the location, Clod surfaces "I looked and couldn't find where X is set" explicitly rather than producing a finding with no anchor.

For Clay: when summarizing findings for Tony, flag location-unknown items explicitly rather than handing them over as if settled. Today's `>` isLiteraL=1 finding was an example — should have prompted "locate this" as a follow-up to Clod before being handed to Tony as a design question.

**Out of scope for current Phase Bytecode arc:** `Parse/BeforeRefactor/`, `Tokf/BeforeRefactor/`, archive directories, `Groups/GUI/`, incantGUI Xcode target.

**Known current state:**
- Bible v2 mirrored across all four repos. Phase naming: Phase Generate Tawk, Phase Integrate, Phase Bytecode, Phase JIT.
- Session 9 closed 2026-05-19. Tawk recon graduated 2026-05-20. Phase Bytecode arc opened 2026-05-21.
- Incant POP fully working as of 2026-05-16: unit-test suite passes clean (pre-Phase-Bytecode baseline at commit eee6a5a).
- `incant` CLI: symlinked at `~/bin/incant` to the Groups debug binary built by TOK Xcode. Standing invocation: `incant <path-to-incant-file>` from anywhere.


---

## Development directives need replace/delete — buffer span vocabulary is the substrate

*Held finding (2026-05-28). Filed so it doesn't evaporate. Not opened as HWF; arc opens when directive work moves past Feature A. Buffer-span-vocabulary is Phase 1.*

**The arc, both ends:**
- **Morning (offline, parked):** buffer/string manipulation — incant can append a field to a buffer but cannot insert a string at an arbitrary point; Buffer has the machinery (`insertIntoBuffer`, `setMark`, `getMarkedString`) but incant can't reach it. Sketch: `setMark` with optional offset, find-string-in-buffer-set-mark, `insertAtMark(GroupItem field)`, remove-at-mark. The pattern: teach Buffer what an incant field is, add incant extern methods to support classes (mirrors Session 9's plg-side support-class extern pattern).
- **Evening (2026-05-28 session's finding):** tok directives are insert-only; development directives (the thing we want — directives that carry real source changes) need replace and delete; replace = remove-span + insert; that is span/extent addressing, not point addressing.

**The convergence:** the buffer span vocabulary (set mark, find span, insert, remove) IS the operation set development directives need. Buffer-knows-what-an-incant-field-is externs ARE the bridge a directive-written-in-incant would call to edit an artifact. The morning's parked buffer work is the foundation layer for development directives, not isolated ergonomics. Same span-vs-point addressing question at two levels.

**Stack, bottom-up:**
1. Buffer gains span vocabulary + incant-field externs (Tony's next offline task).
2. incant directives gain replace/delete, expressed as buffer span ops on the artifact (in incant).
3. Idempotent development — directives have the full edit vocabulary, source changes capturable as reproducible transformations.

**Idempotent-programming through-line:** the goal is source changes expressible as reproducible transformations so the artifact is derivable from `source-plus-transformation`. Hard parts are not the replace operator itself but: edit-vocabulary closure (tractable), determinism (tok mostly has it), and composition/conflict semantics (the tar — overlapping replace/delete spans need resolution; "last wins" or "overlap is error" are legitimate small answers). incant is well-positioned because GroupItem-as-universal-shape and code-is-data mean "transformation over source" is incant operating on incant — homoiconicity cashing out as a practical capability rather than a philosophical claim.

**One leftover feeds the next task:** the DiR dispatch landed clean 2026-05-28 via `head(argument.tag,3)`, but the broader string-matching idiom (`beginsWith` / tag-matching) is exactly what the offline buffer/string pass will design properly. The string facility is the common dependency under both the leftover idiom and the development-directive substrate.

**Not opened as HWF now.** Directive Feature A (insert-only, load-time instrumentation) ships as-is and needs none of this. This arc opens when directive work moves past Feature A, with buffer-span-vocabulary as Phase 1.

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

- [ ] Bytecode.mm → Xcode target (manual: drag into the incant build target, currently named incantGUI — see glossary)
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

### Session 9 follow-up items (2026-05-18 / 2026-05-19)

*Tracking items surfaced during Session 9 (plg debug + actions, incant-mirrored). Each is non-blocking and can land as a standalone brief or be folded into adjacent work. Session 9 closed and graduated 2026-05-19; these items remain as the trailing tasks.*

- [ ] **Stak[] operator** — Stak lacks a by-index accessor. Clod hit this in Brief 3's parseActDeclarations — worked around without it, but the gap surfaced. Small Stak addition in support/Frame. Standalone Clod brief.
- [ ] **PLGmain.twk modification** — accept grammar filename as input, become linkable entry point for generated parsers (e.g., Testing). Brief 8 explicitly deferred main from the class wrapper; PLGmain.twk gets paired with generated `<BaseName>.twk` at link time. Small scope.
- [ ] **Hard-coded include paths in generated output** — plg's generateRules emits `include /Users/anthony/Dropbox/data/InProcess/Include/globals` etc. Tony-specific paths baked into generated files. Portability tracking item; not blocking but worth surfacing for someone else's eventual environment.
- [ ] **Auto-generate class fields from Set/KeyWord declarations** — future plg feature. If action code ever needs per-set field references by name (e.g., `state.excludeSet` rather than `state.setTable["excludeSet"]`), plg would need to auto-generate field declarations in the wrapper class. Not blocking until action code patterns require it.
- [ ] **plg modifier coverage audit vs incant** — modifier table divergence noted: `&` is `isPointer` in incant, `noSkip` in plg. Other modifiers likely overlap but unaudited. Worth a side-by-side audit at some point.
- [ ] **Tok positional directive arg** — `tok File.twk plgDirectives` is the invocation that bakes directives in; `tok File.twk` alone silently produces directive-free output. Failure mode is silent — output looks right but lacks directive injections. Worth a CLAUDE.md note in Parse so anyone re-tokking later knows the second arg is required. Cousin of bible #12: silent staleness in the tok ecosystem.
- [ ] **#PLG and #PLGparse directive blocks unbaked** — plgDirectives has blocks for both that aren't currently active because PLG.twk and PLGparse.twk haven't been re-tok'd with plgDirectives. Bake at next regen of those files. Includes the dumpRules diagnostic from `process "if result"` and the debugRulePLG-flip from `parse return before` — though see next item on the latter.
- [ ] **CLI/env toggle for debugRulePLG** — currently no way to flip the debug flag without a code edit. Clod used a one-line PLG.C patch + revert for Brief 6's Step 3 POP. Two future moves: a CLI flag (`plg -d Testing.g`), or a parse-entry directive once #PLGparse blocks bake. Note the existing `parse return before` directive fires at end-of-parse — too late for parse-time tracing.
- [ ] **Old plg done-summary resurrection** — old plg printed a nice summary on completion. Current `summary()` method exists but isn't at the level Tony wants. Tony works out the spec offline; lands as a brief when ready.
- [x] **Session 9 graduation** — done 2026-05-19. Trim at `Parse/HWFattic/session9plgDebugAndActions.md`. First successful run of the graduation ritual.

### Tawk field resolution recon findings (2026-05-20)

*Tracking items surfaced during the Tawk field resolution recon (Tonto-shaped archaeological recon, first of its kind in the cha cha). Recon document lives at `Tokf/docs/fieldResolutionRecon.md` — read that for full context on any item below.*

- [ ] **InstanceTable.findSymbol probable bug** — `Tokf/InstanceTable.twk:305-326`. Three branches that look like they should `return field.symbol` but actually `return symbol` (the unassigned local). Tonto's headline finding from recon §5.4. Not load-bearing today because `findSymbol` is rarely called — but would bite if usage grows. Tony to confirm vs `use field` semantics he might have overlooked, then fix (or comment as known-broken if the read turns out wrong). Recon section: `Tokf/docs/fieldResolutionRecon.md` §5.4.
- [ ] **foundAncestor shared mutable state** — `Tokf/InstanceTable.twk:241, 248, 262, 270, 280, 292, 298, 170`. `foundAncestor` is a field on InstanceTable mutated during `findInstance` and consumed by `find` after the call returns. Single-threaded non-reentrant assumption is implicit. Resolution doesn't appear reentrant in current call paths, but the contract is fragile. Refactor candidate — see recon §5.1 and §6.2 for the consumer/producer coupling that would need updating in tandem.
- [ ] **fillComponentFields nullInstance negative cache** — `Tokf/SymbolType.twk:564-565`. Negative lookups cached as nullInstance; no invalidation story. Fine if types complete their definition before resolution starts, fragile if anything interleaves type-extension and resolution. Order-of-operations dependency is undocumented; worth confirming before any refactor that changes the ordering. Recon §5.3.
- [ ] **searchForField macro locked-in closure** — `Tokf/InstanceTable.twk:214-227`. The `#searchForField-` macro depends on a four-variable local closure (`field`, `symbol`, `currentLevel`, `lowest`, `last`, `lastParent`). Blocks method-extraction. Refactor candidate — would need return-tuple/struct pattern. Recon §5.2 and §6.8.
- [ ] **checkOverload is 170-line goto-heavy** — `Tokf/Instance.twk:326-500`. Method name suggests yes/no check; behavior is full resolution-and-transformation. Modifies `this` in place. Worth splitting into per-case methods at some point. Recon §5.7.
- [ ] **Method dual-registration in InstanceTable.instances** — methods registered under bare name AND mangled `gitMethodName()` AND for OC, `getOCmethodName()`. Comment on `Tokf/InstanceTable.twk:25-27` flags as known smell. Hazard is theoretical (mangled names contain parens, real collisions unlikely). Worth a refactor if lookup strategy ever consolidates. Recon §5.5 and §6.3.

### Phase Bytecode session findings (2026-05-21, extended through 2026-05-23)

*Items surfaced during the Phase Bytecode design and execution work. Yak shave POP'd through runGenerated dispatch overnight 2026-05-20→21; Brief 3 (gXpress invoke-true) design landed; three incant-machinery questions resolved 2026-05-22; gXpress Option A descent committed 2026-05-23 and parked on listLengtH discriminator question.*

- [x] **listLengtH on `:=`-extracted fields** — investigated 2026-05-23 via three `**` breakpoints; root cause turned out to be a separate bcLIST linkage issue (local bcLIST being populated via setContent rather than bcLIST.group binding to the real bcLIST). Fixed 2026-05-24 overnight. Effect of fix visible in testByteCode (six bcLIST entries) but NOT in testEmitBC or testGXLeaf (still single field xp) — leading to the bcLIST scope-resolution asymmetry finding below.
- [ ] **bcLIST scope-resolution asymmetry across the three tests** — current Brief 3 blocker. emitBC's `:generator bcLIST` reaches the right bcLIST when emitBC fires inside a for-loop body (testByteCode case) but not when it fires outside one (testEmitBC, testGXLeaf cases). Also "two entries per emit" inside the for-loop case is a separate sub-puzzle within the same finding. Tony's seat — incant-machinery, scope-expression semantics around `:generator` resolution.
- [ ] **isLiteraL=1 on operator Tokens** — current Brief 3 blocker. `>` token reads isLiteraL=1 in gXpress, hitting the leaf-emit branch instead of operator-descent. Either legitimate ("matched-as-literal-text" framing — fix is gXpress discriminator change) or misclassification ("value-literal only" framing — fix is upstream in parse). Clod tasked 2026-05-24 with locating where isLiteraL is read in the generate-action layer; Tony reads C++ side once location is pinned.
- [x] **invokE-during-generation gating, first attempt** — broke runGenerated dispatch (`action is: Token` instead of `gIF`/`gBlocK`). Root cause: lazy parsing of generator method bodies happened under the gating flag because the flag was global. Refactored 2026-05-24: `generating` flag moved from GroupRules global to GroupBody per-action; set in generateCode() against the action being generated; TokenXP and ExpressioN actions check via currentMETHOD. Dispatch restored.
- [x] **Three overnight bug fixes 2026-05-24** — (1) runAction double-unwrap removed (runOP already unwraps; runAction was doing it again, breaking generation in subtle ways). (2) printField zero-result handler (dot-op returning 0 was crashing printField; now substitutes falseResult global). (3) bcLIST linkage fix (local bcLIST.group now points at the real bcLIST rather than receiving setContent of it).
- [ ] **Six parse-time errors at gXpress load** — `nextGroup: ERROR true does not contain a list` and `nextGroup: ERROR lines does not contain a list`, three pairs alternating during parse-load of gXpress's definition. Independent of gXpress's body content (errors didn't change when body changed substantially). Doesn't crash the parse. Tony to investigate when convenient — parse-machinery, not blocking.
- [ ] **`**target;` debug breakpoint syntax** — runtime breakpoint primitive. Plants opDebug at runtime; Tony inspects GroupItem in Xcode debugger when execution reaches that line. Worth a CLAUDE.md note and an entry in eventual incant-idioms.md. Today's example: three breakpoints in working-tree generate for listLengtH investigation.
- [ ] **Clay/Clod lane division — Xcode debugger is Tony's seat** — Clod can place `**` breakpoints in source, but inspecting GroupItem state via Xcode debugger requires Tony at the desktop. Pattern: Clod prepares experimental setup, Tony runs the experiment. Worth pinning in bible Working Relationship section. Cousin to "Tony with the C++ debugger" rail named in Phase Integrate.
- [ ] **Bytecode.mm rewrite into tok** — convert hand-written Bytecode.mm to Bytecode.twk source, generate .mm via tok. Matches visibility-gap discipline (source-of-truth tracked, generated artifacts derived) and the mirror principle (incant rule actions live in .twk like their siblings). Off-hours candidate for Tony — familiarity-with-the-runX-handlers payoff alongside the mechanical work. Design calls Tony owns: one .twk for both .h and .mm or split, extern "C" shape, .act-splice pattern adoption. Sequencing: after Phase Bytecode current arc closes; don't blend into active gXpress work.
- [ ] **ifTest modified to exercise gXpress nesting shape** — committed 2026-05-23 to unitTests. Exercises outer if/or/else around inner if/or/or/else inside a for-loop. Durable test-suite value — don't revert. If oneTest's downstream cares about prior simpler shape, that's where any breakage would surface.
- [ ] **Incant style/semantics discoveries 2026-05-23** — six idioms surfaced in one session, all candidates for incant-idioms.md or bible Architecture entry:
  - Field accessors return text-snapshot temporaries, not aliases (`field.taG`, `field.texT` — each access is a fresh temp; mutation doesn't propagate; identity comparison is text-equality only)
  - Cap-on-last-letter convention: `.texT` works, `.text` crashes parse. Convention is "texT=4" in setup.txt
  - No dot-chaining: `.` accesses or tests one attribute, `[]` is the structural accessor. Near-motto: "keep tokens simple." Reason: no incant debugger, short expressions stay reason-aboutable line-by-line
  - C++ GroupField pointers (opFields and family) are not incant-visible. Refer to registries by their incant name: Operators, bcOPs, Grokking
  - Labels-in-labels go as attributes; labels don't have members. `for x in field;` without qualifier walks the right axis for parser-built children
  - listLengtH is a boolean-presence test, not a count comparison. Returns null if no list, count if list present (always > 0 when present). `if X.listLengtH;` is the right idiom; `if X.listLengtH > N;` is wrong-shape.
- [ ] **Incant field semantics — bible Architecture entry pending** — Tony's 2026-05-23 explanation: an incant field is declared in C++ as a pointer but doesn't behave as a C++ pointer. Assignment (`A = B`) copies B's content into A; A and B remain distinct fields. Pointer-shaped storage is structural (uniform field-passing); operations have value-content semantics. `:=` (opSetGroup) is the group-binding variant, distinct from `=` (opAssign, value-copy). This is a foundational concept that several idioms follow from. Worth bible Architecture entry alongside "Incant Core Concept" and "Incant Dispatch Idiom" — current draft in this session's discussion. (Bible draft pending in next session.)
- [ ] **Brief verification rigor — durable lesson** — "Shape-reading isn't verification; running and sifting the bones is." Surfaced 2026-05-22 (testGXLeaf "passing" turned out to be lucky-coincidence pass-through twice — once via empty-bcLIST, once via single-Token-ExpressioN fallthrough in gXpress). Cousin of TODO's existing "Brief 2 verification too loose" lesson. Worth bible Working Relationship section at next refresh.
- [ ] **incant-idioms.md as a doc — sequencing** — TODO has it as a Clod-discovery destination. Today (2026-05-23) accumulated enough idioms to warrant drafting v0. Proposed structure: (1) bible Architecture entry for field semantics — the foundational concept (`=` vs `:=`, pointer-storage-vs-value-semantics); (2) incant-idioms.md for the consequences (snapshot accessors, no dot-chaining, registry-by-name-only, listLengtH as boolean test, runAction parameter binding quirks, etc.). Draft when Phase Bytecode current arc closes; not now (momentum).

### Phase Bytecode session findings (earlier items)
 — resolved 2026-05-22 (Tony work). Root cause: incomplete definition of the group field `isLiteraL` itself, not a setContent/opDot late-binding issue as initially suspected. With the definition completed, righty no longer picks up isLiteraL spuriously.
- [x] **opDot late-binding unwrap unitTest failures** — resolved 2026-05-22 (Tony work). Root cause: the unwrapping code added to opDot was unnecessary. `runOP()` (the caller of opDot) had already done the unwrapping; the second unwrap in opDot never hit during normal generator runs. Removed from opDot. UnitTests pass clean.
- [ ] **argument[N] on +=-stored children** — Q3 of Clod's Brief 3 findings. `xl` built with `xl += op; xl += target; xl += arg;` — accessing children from incant via `argument[1]` returns falsy. The `[N]` accessor works for parser-built children (gFOR uses argument[1] on FOR rule and works) — difference is `+%` vs `+=` storage. Needs different accessor for `+=`-added children, OR `[N]` needs to walk `+=` storage too. May be related to invokE getter / opDot chase question.
- [ ] **emitBC parameter naming convention** — `runAction` (GroupRules.mm:2933) only binds to a parameter literally named `argument`. emitBC's previous `operand` parameter was getting empty values because of this. Worth a CLAUDE.md note in incant so future generators don't hit this. Cousin of bible #12 (silent-staleness in tok ecosystem) — code looks right, runs without complaint, but parameter binding silently empty.
- [ ] **Brief 2 verification too loose** — Lesson: Brief 2's "verification" passed with both bcLIST entries empty (due to the emitBC parameter-binding bug). The loose-verification bar let a real bug ship in the commit. Worth thinking about: when verification is loose, what trust does that buy, and what doesn't it buy? Lesson candidate for cha-cha-assessment beat or bible Working Relationship section.
- [x] **ElsE forward-reference grammar fix** — resolved 2026-05-22 (Tony work). The if/or/else pattern error was caused by a missing rowradr declaration of the ElsE rule in the incant grammar. With the declaration added, the three-way if/or/else chain works correctly. Worth a CLAUDE.md note for future grammar-rule additions: "grammar rules referenced before their full definition need a rowradr forward declaration."
- [ ] **Three-way if/or/or chain in generator actions** — Clod's Brief 3 first blocker. Resolved by the ElsE forward-ref fix. Worth confirming via a deliberate three-way-chain test in unitTests so the regression-protection is durable. Existing testInterpret uses two-branch only.
- [ ] **`=:` operator design** — future grammar-change session. Goal: collapse `string` keyword. `whatsIt =: 'is what I am talking about':;` — assignment with string-build semantics, using print machinery for shortcut+expression handling on the RHS. Open design question: where `=:` lives in grammar (Path A: operator inside ExpressioN with sub-grammar switch for RHS; Path B: separate statement form parallel to PrinT). Tony favored Path A direction (preserving assignment-flavor); plg conditions may be the parser-switch mechanism. Not blocking gPrint bytecode work; parallel design.
- [ ] **Grammar-change discussion template** — Tony has multiple grammar ideas percolating beyond `=:`. Worth developing a standard format for these discussions: what's the current grammar, what's the proposed change, what's the minimal-impact path, what ripples to design around. Lands as `Groups/docs/grammarChangeTemplate.md` or similar when format settles.
- [ ] **Print bytecode plan document** — `Groups/docs/printGenerationPlan.md`. Drafted in chat during 2026-05-21 woodshed. Pending: write up as durable artifact, fold in `=:` interaction notes. Covers: print's interpret-time semantics (aCTionPrinT + appendGroup + printField), grammar handoff (PrinT/StringXP/PrintXP/FormaT), bytecode design (bcPrintShortcut/bcPrintField/bcPrintEmit/bcStringEmit), brief sequencing for gPrint implementation. Substantial enough to warrant a session of its own.
- [ ] **Three cha-cha modes pinned** — woodshed (Clay+Tony, plan-shape work), brief-execute (Clod, mechanical-shape work), commander-discretion (Tonto, archaeological-shape work). All three demonstrated in practice during 2026-05-19/20/21 sessions. Worth marking in bible Working Relationship section at next refresh. Pattern surfaced naturally during Tawk recon (first commander-discretion) and Phase Bytecode briefs (brief-execute formalized further).
- [ ] **Avoid duplicate unwrap across caller/callee** — design heuristic surfaced during the opDot resolution. Before adding unwrap code in a consumer (opDot), check whether the caller (runOP) is already unwrapping. The original "fix consumer not producer" framing held, but the specific opDot unwrap turned out to be the duplicate-and-removable one because runOP already unwrapped upstream. Worth pinning: when nested-group handling is in play, audit the full call chain for who unwraps where before adding new unwrap code.
- [ ] **Incant by Clod-discovery as documentation pattern** — Lessons section observation. Clod's findings about incant idiom (canonical-vs-instance, setContent semantics, scope expressions, runAction parameter binding) become durable documentation when captured. Worth eventually folding into an incant-idioms.md in `Groups/docs/` once enough findings accumulate. Not yet, but the pattern is real.

### Bible refresh — minor sync passes (after major arcs settle)

*The bible v2 from 2026-05-15 is substantially current. Small drift items accumulate:*

- [ ] Session 6 (parse error handling) — add to bible's HWF Sessions Pending Work index when refresh happens
- [ ] PLG self-host status — currently hedged "unknown until next attempt." A future Tonto run could confirm cheaply. Worth doing during a low-stakes Tonto window.
- [ ] PLG Next items status pass — happens when Phase Integrate brings us back deep into plg work

### PLG — Self-hosting

- [ ] Action blocks feature
- [ ] Grammar reorganization
- [ ] **Paren-alt decomposition for incant** — port BlockplgAct from PLG. Reference design is the PLG implementation. Low priority.

### Phase Integrate — extended

- [ ] TAWK autopsy remainder (after Phase Integrate completes)
- [ ] Scoped TAWK autopsy (independent): GC inheritance fix, include guard fix — go into legacy Tokf/Tawk.twk directly

### TOK Xcode project — yaml it (+ rename Groups → incant, rename incantGUI target → incant)

*Lives outside all four GitHub repos. No project.yml. Reverse-engineering from existing .pbxproj is the work. May also include renaming target. Housekeeping for whenever. The "incantGUI" target name is vestigial from when GUI work was active — see bible glossary.*

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

- [x] **GroupRules.twk restored as source of truth (2026-05-28)** — 2026-05-26 .mm hand-edits now fully reproducible from bare `tok GroupRules.twk`. Triage: ~37/40 hunks were regeneration lag (.rtn already ahead of stale .mm); 2 functions (`applyDirectives`, `spliceDirectives`) moved .mm→Instruct.rtn as native source; `groups.ext` externs added; `+=` DiR dispatch landed in `opPlusEQ` (`head(argument.tag,3)` match → `applyDirectives`); debug scaffolding stripped to `groupDirectives`. Directive round-trip route explored, set aside as unnecessary (changes already in .rtn). Surfaced the tok-directives-insert-only finding.
- [x] **Wiki first-pass restructure (2026-05-24)** — What-Is-Incant page revised: bootstrap section moved to Appendix B with framing intro, For the Nerds moved to Appendix A, JSON/YAML progression moved earlier in the flow (placed right after Foundation: What Is a Field?). The reflexive/homoiconic incant-grammar example folded into "What Is a Rule?" where it earns its place. Tone preserved. Weekly cadence proposed for ongoing wiki refinement. Features+working-examples list as next destination (rather than single illustrative program). Pushed to incant.wiki by Clod 2026-05-24.
- [x] **Per-action `generating` flag (2026-05-24, Tony work)** — Refactored from GroupRules global to GroupBody per-action. Set in generateCode() against the action being generated; TokenXP and ExpressioN actions check via currentMETHOD. Fixes the dispatch regression caused by the first gating attempt (which broke runGenerated because lazy parses of generator method bodies happened under the global flag). Clean fix; no other implementation changes needed.
- [x] **Three overnight bug fixes (2026-05-24, Tony work)** — runAction double-unwrap removed (runOP already unwraps); printField zero-result handler (substitutes falseResult global); bcLIST linkage fix (bcLIST.group points at the real bcLIST rather than setContent copy).
- [x] **Cha-cha finding-location practice pinned (2026-05-24)** — Clod's reports include file/method/line/conditions for generator-side findings. C++ layer stays in Tony's lane unless explicitly delegated. Clay flags location-unknown items explicitly rather than treating them as settled.
- [x] **ifTest modified to exercise gXpress nesting (2026-05-23)** — outer if/or/else around inner if/or/or/else inside a for-loop. Falsified Clod's nesting concern via standalone run. Durable test-suite value for the gXpress shape going forward.
- [x] **runAction unwrap-at-parse-time bug fixed (2026-05-23, Tony work)** — runAction was setting argument before code parse, so argument got unwrapped at parse time and the value (not the parameter reference) got baked into the code. Fixed. Didn't entirely fix runGenerated (something is still pointing at a stale argument there), so the `dummy = argument; action(dummy)` hack at runGenerated lines 201-205 is the residual workaround.
- [x] **Incant style/semantics discoveries surfaced (2026-05-23)** — six idioms pinned for future incant-idioms.md draft. See Phase Bytecode session findings for the full list.

- [x] **Incant bytecode short-doc written and pasted atop XML/WorkingOn/generate (2026-05-22)** — structure, registries (bcOPs and Operators), emit-side mechanics, Generating registry layout, and a paragraph contrasting incant bytecodes (GroupItems, attribute-lookup dispatch, interpreter writable in incant) with standard bytecodes (opaque tuples, switch-decode). Surfaces the bcPushLit/bcPushField/bcStoreField/bcMul registration gap as an explicit open question. Drafted by Clay in 2026-05-22 morning session in response to Tony's question about `bcPushField`. Pasted as comment block at top of generate file.
- [x] **Three incant-machinery investigations resolved (2026-05-22, Tony work)** — righty/isLiteraL (incomplete group-field definition), opDot late-binding unwrap (unnecessary code in opDot, removed since runOP already unwraps), if/or/else pattern (missing rowradr declaration of ElsE rule in incant grammar). All three closed the gating set for Brief 3 verification.
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
