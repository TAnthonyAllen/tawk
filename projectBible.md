# The PLG/TAWK/Incant Project Bible
*A living document. Every sentence earns its place.*

---

## The Ecosystem in One Breath

**PLG** finds the pieces — a fast, declarative pattern matcher. Domain-specific, stateless, embeddable.
**TAWK** shapes the expression — a source-to-source transpiler. Reads tawk syntax, outputs C++.
**Incant** understands the meaning — reflexive, homoiconic, stack-aware, self-describing.

*PLG recognizes. TAWK transforms. Incant reasons.*

The three are one ecosystem. Working on any one inevitably involves the others.
The design follows elegance; the classifications are just other nerds recognizing what was already there.

---

## Repositories

| Repo | URL | Status |
|------|-----|--------|
| PLG | https://github.com/TAnthonyAllen/plg | public ✅ |
| Incant | https://github.com/TAnthonyAllen/incant | public ✅ |
| Support | https://github.com/TAnthonyAllen/support | public ✅ |
| TAWK | https://github.com/TAnthonyAllen/tawk | public ✅ |

Each repo has `CLAUDE.md`, `TODO.md`, and this bible mirrored.

---

## Local Directory Map

```
/Users/anthony/Library/CloudStorage/Dropbox/data/
    InProcess/
        Parse/              — PLG repo root. All active plg source lives here directly.
        Parse/.git          — plg repo
        Parse/Grammar/      — Grammar files: plg.g, plgRules.g, plg.act
        Parse/Backup/       — parked legacy plg material (gitignored)
        Parse/BeforeRefactor/ — historical pre-refactor copies (gitignored)
        Parse/Tests/        — test files + symlinks (gitignored)
        Parse/plgDirectives — active debugging-directive file (untracked, kept in place)
        Tokf/               — TAWK source (tawk repo)
        Tokf/Tests/         — TAWK test sandbox (gitignored)
        Tokf/Tawk.regen.twk — PLG-generated setRules in new format (gitignored artifact)
        Groups/             — Incant source (incant repo)
        Include/            — symlink → ~/data/support/Include
        Frame/              — symlink → ~/data/support/Frame
        Groups/Maps         — symlink → ~/data/support/Maps
    InProcess.xcworkspace   — umbrella workspace containing all projects
    support/                — shared support classes (support repo)
        Frame/              — Buffer, DoubleLinkList, PLGset, CharSet, BaseHash, Stak, Tape, StringRoutines
        Include/            — TAWK externals
        KeyTable/           — keyword lookup
        Maps/               — BitMAP, Segment
```

**PLG directory flatten**: Parse/ was previously a parent containing Parse/Revision/ as the active codebase. On 2026-05-15 the Revision/ contents were promoted to Parse/, legacy material moved to Parse/Backup/, and .git relocated to Parse/.git. All plg source now lives directly in Parse/.

**Symlink architecture**: shared support classes live once in `~/data/support/`; the InProcess paths (`Include/`, `Frame/`, `Groups/Maps`, etc.) are symlinks pointing into the support repo. This keeps existing `.twk` `include` directives working unchanged while making the support repo the single source of truth. Edits to a shared class in either path land in the same file.

**APFS case-insensitive gotcha**: `plg.twk` and `PLG.twk` are the SAME file on macOS. Generated artifacts must live in a subdirectory. Grammar/ was created specifically to avoid this collision.

**PLG regen output**: `PLG.process()` writes to `<base>.regen.twk` in the directory where plg was invoked (CWD). When running on Tokf/Tawk.g from Tokf/Tests, output goes to Tokf/Tests/Tawk.regen.twk — NOT Tokf/Tawk.twk (the real source). plg's contract is "this file here, output here" — invocation-directory-relative, not source-file-resolved.

**Visibility-gap discipline**: source-of-truth files must live in tracked locations. The PLGrgx case (untracked in legacy Parse/, moved into plg repo as pre-flatten step, commit 139064b) and the PLGset case (untracked in legacy Parse/, moved to support/Frame as sister of CharSet) both exemplify the fix. Untracked source-of-truth invites the commit-vs-disk drift that the sign-off ritual (HWF Session 2) is meant to catch.

---

## Language Classifications

| Project | Type | Key Properties |
|---------|------|----------------|
| PLG | DSL / pattern matcher | Declarative, stateless, fast, embeddable |
| TAWK | Transpiler | Source-to-source, metaprogramming, syntactic sugar for C++ |
| Incant | General purpose | Reflexive, homoiconic, stack-aware, will be JIT enabled |

**DSL** — Domain-Specific Language. Designed for one purpose.
**Reflexive** — the language can examine and describe itself.
**Homoiconic** — code and data have the same structure. A GroupItem field IS the rule that describes it.
**Transpiler** — compiles to source code (C++) rather than machine code.

---

## Architecture

### PLG Core Classes (new architecture)
- **PLGparse** — base parser. Buffer-based input, cursor/eof, rules/setTable hashes, addTest(), getRule(), getSet(), parse(), divertInput/revertInput stack. Library citizen only — no `main()` of its own as of 2026-05-15.
- **PLGmain** — class wrapper that owns the program `main()`. Separates library role (PLGparse) from program entry point. Without this split, linking against PLGparse drags PLGparse's old `main()` in unexpectedly.
- **PLGrule** — one grammar rule. Has alternatives (DoubleLinkList), guardSet, action callbacks (defer/immediate/fail).
- **Alternative** — one option within a rule. Has elements (DoubleLinkList), guardSet.
- **Element** — one match unit. Kind (kLit/kChr/kSet/kAny/kEof/kRuleRef/kKeyTable/kCondition/kVariable/kUpTo/kBalanced), min/max, setRef/litText/ruleRef etc.
- **PLGitem** — match result. Buffer reference + offset, itemLength, toString().
- **PLG** — contains PLGparse via composition. setRules() and all plg-specific grammar/actions.

### Support Classes (in support/Frame, sister citizens)
- **PLGset** — parser-rule set class. Uses `inSet[256]` representation. Lives in support/Frame as of 2026-05-14 (commit 8223af6), resolving months of source-of-truth ambiguity.
- **CharSet** — character-set utility. Same `inSet[256]` representation. Sister of PLGset.

### Key Design Decisions
1. **Buffer-based input** — all input lives in Buffer objects. divertInput pushes/pops buffer stack.
2. **Safe iteration** — `for (link = list->first; link; link = link->next)` — never add manual advance inside body (TAWK's for loop: for link = list.first; already advances — double-advance = SIGSEGV).
3. **Composition over inheritance** — PLG contains PLGparse field (workaround for TAWK header issues).
4. **toString() only** — PLGitem string() and unString() that generate temporary strings in the input stream are not used (they were in old plg).
5. **addTest() shorthand** — `addTest(kind, data, label, min, max, skipSet)` creates and wires an Element in one call.
6. **Guards** — setGuard() computes FIRST sets. CRITICAL: null guard = accept anything; empty PLGset = reject everything. These are NOT the same. When setGuard() can't determine FIRST set (kAny, negated sets, kEof etc.) return null, not empty set.
7. **Method ordering** — alphabetical by convention (Anthony's preference). Not a TAWK requirement.
8. **extern "C" bridge functions** — functions bridging two types: `foundIn(PLGset, PLGitem)` is the example.
9. **Two-table pattern** — process() swaps rules/setTable to fresh BaseHashes before parse. Callbacks build into fresh tables.
10. **TAWK error propagation** — unresolved references embedded as errors in generated C++. Feature — errors appear in context.
11. **Library/program separation** — PLGparse stays a library citizen; PLGmain owns `main()`. Anyone linking against PLGparse picks up no unexpected entry point.

### The Match Chain
```
PLG::parse(name)
  → PLGrule::match(state)
    → Alternative::match(state, out)
      → Element::match(state)
        → Element::matchByKind(state)
          → matchSet / matchLit / matchRuleRef / etc.
            → Element::applyRepetition(state)
```

### Guards
setGuard() computes FIRST sets recursively with cycle protection via `guardComputed` flag. Zero-advance-stop: if a rule succeeds without consuming input, treat as failure.

---

## The Bootstrapping Problem

PLG is supposed to generate setRules() but needs setRules() to parse plg.g to generate setRules().

**Current bootstrap sequence:**
1. Old PLG binary runs on `Grammar/plg.g` → generates plg.twk
2. `python3 translateSetRules.py plg.twk new_setRules.twk` → new format
3. Paste setRules() into PLG.twk
4. `tawk PLG.twk` → PLG.C
5. Compile and test

**Self-host status**: plg.g parses end-to-end (39 rules). Generator emits correct addTest() format (76% smaller). Round-trip chain stalls at bare-include over-matching — next debug cycle, use plgDirectives.

**Same problem exists for TAWK**: solved by `~/bin/tok`.

---

## PLG Grammar File Architecture

Grammar source in `Parse/Grammar/` (post-flatten path):
- `plg.g` — rule structure. 4 changes from original: plgRules.g include, action.g excluded, bare-include support, Start rule fixed to `Header* '%%' Body+ '%%'? Trailer?`
- `plgRules.g` — shared foundational rules (Comment+CommentBody, Integer, Name, Label etc.)
- `plg.act` — action code
- `action.g` — DEFERRED pending Action-blocks feature

**Set declaration format**: All Set declarations require a `;` terminator: `Set mySet [...] ;` — not optional. This prevents over-matching when empty Sets are followed by tokens that look like bare Names.

**Quote character gotcha**: Matching `'` or `"` inside PLG grammar. Options: (1) use the other delimiter — `"'"` matches single-quote; (2) set notation `[']` — always unambiguous. The `[']` approach preferred when both quote types need matching in same grammar.

**Action blocks design** (approved, TODO):
```
Action actionName { TAWK body } ;
```
Generates: `void actionName(PLGparse *state, PLGitem *item) { body }`
Goal: grammar files self-contained, no separate .act/.rtn files needed.

---

## Incant Grammar

Incant has its own grammar, parsed by the incant runtime (not by plg). The grammar is bootstrapped: 32 hard-coded rules in the runtime bootstrapper create the rules and fields incant needs to define and run a rule, and the rest of incant is loaded from a setup file containing further incant definitions.

**Note on terms**: a rule is a field, and a field can be a rule. Fields can also be data, methods, containers, and other things that aren't rules.

### User-facing source rules

**`:` and `>` are checkSkip-internal, never user-facing**: incant source must not contain `:` or `>` characters typed by the user. These are checkSkip's synthetic output for marking block boundaries inferred from indentation. A user who types them in incant source triggers silent parse failure — no error posted, parse craps out invisibly. This is a "hurts when I do this" problem with an obvious mitigation. The broader silent-failure issue is queued as HWF Session 6 (parse error handling).

**Stacked-terminator runtogether**: At structural boundaries where two terminators stack — the canonical case is `;;` at the end of a `define` block, where the first `;` ends the last definition and the second `;` ends the `define` command itself — the terminators must be adjacent. No whitespace between them. Whitespace at that boundary causes checkSkip to read the gap, infer an open block needs closing, and synthesize a phantom `>`. The parse then craps out silently per the rule above. Concrete shape: `Start=StatemenT+;;` is well-formed; `Start=StatemenT+; ;` is not.

### Recent changes

**Indent-as-structure [HWF Session 4, settled, implementation in progress]**: incant has adopted indentation-as-structure for both definitions and code blocks. Colon opens a block, dedent closes it, single `;` terminates statements. This replaces the old stacked-`;;;` count-close mechanism — dedent does the structural work. The win is twofold: incant is cleaner to read, and the language is pushed further toward its homoiconic character, with definitions and code now sharing the same visual structural conventions. Fork A implementation (checkSkip injecting `:` and `;` from indentation) is partially landed and being hardened through the current debugging cycle; the user-facing source rules above are the residual constraints from that work.

**Code-as-member-shape [HWF Session 4, decision D]**: action bodies have migrated from end-of-line trailers attached to field declarations to a `{code}` member shape that lives inside the field body alongside other members. Attribute-name labels the binding at point of declaration — `onLayout: {code};`. Code is stored in the CodE attribute (noPrint disposition: fire-and-forget at define time). This decouples code from positional attachment and allows multiple action-bearing members per field.

**CodE/DatA atomic parseAction [2026-05-14, commit a15471c]**: code-block field values of shape `name = { ... };` are parsed atomically through aCTionCodE, bypassing checkSkip's indent-state machinery. This is the implementation surface that makes code-as-member-shape work in practice.

**checkSkip indent-mode hardening [2026-05-15]**: the double-define bug fix landed today. The two user-facing source rules above (`:`/`>` non-user-facing, `;;` runtogether) are the residual constraints that emerged from this fix.

**Bootstrapper rules — evolving**: the 32 hard-coded bootstrap rules have undergone recent changes during the indent-as-structure work and surrounding grammar tightening. Specifics live in the live grammar file (see Parse/) rather than being mirrored here — the bootstrap set changes too often during this stretch of work to be worth pinning in the bible.

### Pending grammar changes

The incant grammar is in active evolution. Resurrection-readers should treat this section's specifics as "current state, not final state" and check HWF.md for sessions reshaping these rules. Known pending work includes:

- **Indent-as-structure full implementation (HWF Session 4, Fork A)** — the Fork A grammar design is settled but Phase 1 (GroupMain.twk grammar polish + DEFINing flag promotion) and Phase 2 (GroupRules.twk checkSkip additions for defining state) have not been fully executed. Migration of existing incant code from structural `;;` / `;;;` to single-`;` plus indentation is a separate arc.
- **Bootstrap definition rule changes** — upcoming work on documentation.md surfaces (a pending conversation, see TODO) is expected to drive changes to how definitions are bootstrapped.
- **Paren-alt decomposition for incant** — port of PLG's `( A | B )` decomposition machinery (BlockplgAct, recursive parent-prefix naming) to the incant grammar. Not urgent. Reference design is the PLG implementation.
- **Parse error handling (HWF Session 6)** — no design yet. The current silent-failure behavior on bad input (see user-facing source rules above) is the trigger.

---

## Paren-Alt Decomposition (BlockplgAct)

When a grammar rule contains inline `( A | B )` syntax, PLG decomposes it into a named helper rule during parse.

**Algorithm:**
1. `BlockplgAct` fires as a deferred callback when `Block` rule matches
2. Creates helper rule named `<parentRule>Block<N>` where N is a per-rule counter (resets per rule)
3. Recursive nesting: inner blocks use the helper as parent — `QuotedStringBlock0Block1`
4. All modifiers (label, min/max, noSkip, banged) transfer to the kRuleRef pointing at the helper

**Naming convention**: Recursive parent-prefix — `QuotedStringBlock0` → `QuotedStringBlock0Block1`. Counter resets per rule to prevent monotonic growth and name collisions.

**Important**: `( A | B )` cannot be stripped from PLG — TAWK's grammar uses it 5 times including nested forms. The decomposition machinery is required for TAWK ingestion. (Note: porting this machinery to incant grammar is on the pending list — see Incant Grammar / Pending grammar changes.)

---

## TAWK Known Issues (autopsy table)

1. **Empty `//` comment lines** reset field resolution context. Fix: context tracking should be comment-blind.
2. **Implicit field resolution** complex. TAWK refactor: `@field` context markers.
3. **No include guards** in .h files. Add manually for inherited classes.
4. **`new` type inference** inconsistent. Workaround: `field = new ClassName()`.
5. **No `#include` in .h files** — add manually after every re-tawk.
6. **Unused field** — TAWK silently drops. Fix: emit warning, keep field.
7. **`kSet`/`kRuleRef` macro conflict** — test macros, not assignable/comparable values. Use numeric literals.
8. **`extern "C"`** in tawk-generated files gets clobbered on re-tawk. C-linkage functions in hand-written files.
9. **No include search paths** — all includes must be absolute paths.
10. **TAWK iteration trap** — `for` loop already advances. Never add manual advance inside body.
11. **TAWK Directives** — debug injection without source pollution. `tawk filename directiveFile`. Directive files exist for PLG, Incant, TAWK. Use BEFORE adding print statements to source.

---

## Incant / PLG Convergence

- `match()` — atomic recognition. `parse()` — semantic processing.
- In PLG these are separate. In Incant they collapse — data structure IS grammar IS result.
- **defer**: in PLG a callback convention. In Incant a first-class keyword. Same insight.
- **Labels**: isLabel = parse result, same name as rule that produced it. isRule = rule definition. Everything inside label context is isLabel.
- **The north star**: PLG written in Incant.

---

## TAWK Runtime Replacement (Phase Integrate)

TAWK currently uses the legacy PLGparse API (kind=5/7 with void* casts, *TawkNow callbacks). New PLG generates the new API (kind=6 with bare strings). Bridging this gap is the Phase Integrate arc.

**Phases:**
- A: Tawk.twk setRules() — splice Tawk.regen.twk body into legacy source
- B: Port ~50+ *TawkNow/*TawkAct callbacks — signatures change to (PLGparse state, PLGitem iTEM); children via iTEM.children["label"]
- C: First clean compile
- D: Tests/ sandbox verification vs ~/bin/tok
- E: Promotion

**Precondition**: Phase Integrate requires a working incant interpreter as its baseline. As of 2026-05-15, incant POP runs to completion and a test action fires end-to-end, but the full unit-test suite has known gaps. Phase Integrate planning (Tonto recon) proceeds, but execution against a real Tokf migration waits on incant unit-test cleanup completing first.

**Safety rules:**
- Tests/ directory in Tokf/ is mandatory sandbox — never touch Tokf/Tawk.twk before Tests/ proves it
- ~/bin/tok is the safety net — broken TAWK can't fix itself

**Independent of this arc:** Scoped TAWK autopsy (GC inheritance + include guards) goes directly into legacy Tokf/Tawk.twk.

---

## Resolved Issues Log

*For orientation — issues that seemed like blockers but were resolved.*

- **PLG directory flatten [2026-05-15]** — Parse/Revision/ promoted to Parse/, legacy material parked in Parse/Backup/. Commit ae06990. Build clean for core code; cosmetic xcode-navigator cleanup deferred to yaml-refresh work.
- **checkSkip double-define bug [2026-05-15]** — testCodE parsed correctly through aCTionCodE, then parser rewound and re-defined the same field. Root cause: checkSkip's synthetic-`:` insertion in indent-mode created corrupted input, triggering rewind-and-replay. Fixed. The stacked-terminator runtogether rule (`;;` with no whitespace) is a residual user-facing constraint from this fix — see Incant Grammar.
- **PLGmain split [2026-05-15]** — PLGparse's `main()` extracted into a new PLGmain class wrapper. Linking against PLGparse no longer drags in unexpected program entry point.
- **CodE/DatA atomic parseAction [2026-05-14, commit a15471c]** — code-block field values parsed atomically through aCTionCodE, bypassing checkSkip indent-state issues.
- **PLGset source-of-truth resolution [2026-05-14, commit 8223af6]** — PLGset moved from untracked legacy Parse/ to tracked support/Frame, sister to CharSet. Resolved months of ambiguity.
- **Bare-include over-matching** — STALE FRAMING. Was never actually broken. Directives trace proved Body was working correctly. The old session notes were based on pre-fix state.
- **expression.g 0.6% coverage** — Java-style block comment at file head tripped CommentBody. Fixed by pre-parse stripComments() replacing in-grammar comment handling.
- **keywords.g NOTE pollution** — `//` line comments not recognized by meta-grammar. Word "NOTE" parsed as rule name, eating downstream KeyWord declarations. Fixed by stripComments().
- **parts.g ElementType {N,M} stop** — PLG.twk:900/909 had empty string literals where `{` and `}` belonged. Hand-coding placeholders never filled in. Two-char fix.
- **Word bootstrap encoding** — PLG.twk:740/744 had `"n;"` instead of `"^ \f\r\t\n;"`. Hand-coding error causing Word to only match `n` or `;`. C compiler escape processing handles the fix correctly.
- **SetVariable zombie rules** — SetVariableplgNow not wired (commented out). Sets registered as empty Rules instead of in setTable. Fixed by porting and wiring SetVariable dispatcher.
- **Tawk.twk silent overwrite** — PLG.process() was writing `<base>.twk` to input directory. Running on Tokf/Tawk.g overwrote Tokf/Tawk.twk. Fixed: output now goes to `<base>.regen.twk`.
- **APFS case collision** — `plg.twk` and `PLG.twk` same file on macOS. Solved by Grammar/ subdirectory.
- **plg path resolution** — process() was resolving the real path of input and writing output next to that, instead of writing to the invocation directory (CWD). Symlink invocations from Tokf/Tests broke includes lookup with empty sourceDir. Fixed: process() simplified (17 lines removed, 1 added), output now CWD-relative. Regen content unchanged across the fix (MD5-identical to baseline). Determinism preserved.

---

## Incant Bytecode/JIT

Incant bytecode/JIT design work has its own document — see `jit.md` (sibling to this bible). That document covers the bytecode-as-GroupItems design, the bcOPs registry, the interpret() function written in incant, and the gating hook in GroupRules.mm.

**Status summary [updated 2026-05-15]**: Phases 0 (BDWGC) and 1 (generateCode repurposed) complete; Phase 2 (bytecode emitter + interpreter) in progress. Precondition for Clod-driven emitter work: incant unit-test suite passing clean. Pointing Clod at emitter rewrites while the interpreter has gaps means a failing testByteCode run can't be attributed cleanly to emitter vs interpreter — and Clod cycles would burn on phantom bugs.

### Incant Dispatch Idiom (IMPORTANT)
Two steps — never chain:
```
handler = field.attribute;    // get the attribute
handler(argument);            // call its method
```
One method per field by design. Sub-attribute pattern for second invokable behavior.

---

## The Long Game — Incant as Distributed Virtual OS

GroupItem fields are deployable units. Run anywhere. Message each other across platforms. Location transparent.

Claude is a GroupItem — `isCLAUDE` alongside `isSTRING`, `isNUMBER`, `isGROUP`. The AI is not a tool called from incant — it IS a field in incant. (HWF Session 1 is unpacking what this means in practice — see HWF.md.)

Go-style channel messaging — steal Go's goroutine/channel pattern. HPDL. Ken Thompson approved.

ZFS-flavored storage — copy-on-write, snapshots as GroupItem operations. HPDL.

The JIT is the enabling technology. Without JIT, incant is an interpreter. With JIT, incant ships.

---

## Priority Plan (current)

**Phase 1: PLG → Tawk.g ✅ COMPLETE**
- PLG parses plg.g ✅ — 39 rules
- PLG parses Tawk.g ✅ — 200 rules, 177 populated
- Tawk.regen.twk generated in new format ✅

**Phase Integrate: TAWK Runtime Replacement (multi-session arc)**
- Recon, file-by-file migration of Tokf/, build ~/bin/tokTemp, validate, promote
- Precondition: incant unit-test cleanup
- See "TAWK Runtime Replacement" section for phase detail

**Phase 3: Incant bytecode/JIT**
- Emitter rewrite (gIF, gExpressioN)
- testByteCode POP
- Expand from there
- Precondition: incant unit-test cleanup (for Clod-driven work)
- See jit.md for design detail

**Incant unit-test cleanup** — load-bearing precondition for both Phase Integrate execution and Phase 3 Clod work. POP runs to completion as of 2026-05-15 and a test action fires; full suite has known gaps. Step-by-step work-through is Tony's after-hours bucket.

**Scoped TAWK autopsy (independent of Phase Integrate)**
- GC inheritance fix — TAWK classes need to inherit from GC
- Include guard fix
- These go into LEGACY Tokf/Tawk.twk directly

**Longer term TAWK autopsy** — HPDL. Plan carefully — many .twk files need re-tokkifying.

---

## HWF Sessions — Pending Work

The HWF.md file holds active design sessions. The following are pending or active as of 2026-05-15:

- **Session 1 — isCLAUDE and the cha cha** — open. Working through what `isCLAUDE` means as a GroupItem field type.
- **Session 2 — Sign-off ritual** — queued, origin captured. Cross-session verification design.
- **Session 3 — GUI / XML / incant role** — queued, origin captured. XML-to-incant conversion rules.
- **Session 4 — Indentation as structure** — open. Major incant syntax design; Fork A implementation partially landed and being hardened. Indent-as-structure (definitions and code blocks) and code-as-member-shape both settled and reflected in Incant Grammar above.
- **Session 5 — PLGset / CharSet architectural split** — substantially settled (PLGset/CharSet placement landed 2026-05-14); session-close audit pending.
- **Session 6 — Parse error handling** — queued, not yet opened. Triggered by the `:`/`>` silent-crap-out problem and the broader observation that incant has no parse-error story yet. Bounded mitigation in place (don't type those characters); proper design work deferred.

See HWF.md for active session content. Bible carries the index so resurrection-readers know what's queued.

---

## Working Relationship

**Anthony (Haps)** — architect, domain expert, final authority.
**Clay** (Claude at claude.ai) — design, reasoning, architecture, HWF navigation.
**Clod** (Claude Code) — execution, file edits, GitHub, build verification.

**Standing permissions**: Clod changes any code in source directories without asking. Ask before GitHub pushes. No option menus — do the needful.
**Clod protocols**: "got it" when message lands. "ready" when done. PLG:/Incant: labels when parallel tracks.
**End-of-session ritual**: Clay drafts bible + TODO, Clod pushes to all 4 repos. Before every Goodnight Gracie.

**Resurrection-reader standard**: All .md files in this project (bible, HWF.md, TODO, CLAUDE.md, etc.) must make sense to fresh-Claude reading them cold tomorrow with no memory of today. The .md files exist to make resurrection work — reading them is how Claude/Clod start each day, and that reading is how project continuity persists. See HWF.md preamble for the full statement. This is the primary writing standard for project documentation.

---

## Current State (last updated: May 15 2026)

**Deltas since previous mark (May 7):**
- plg directory flatten landed (Parse/Revision/ → Parse/)
- PLGset / CharSet placed as sisters in support/Frame
- PLGrgx tracked into plg repo
- CodE/DatA atomic parseAction approach committed
- checkSkip double-define bug fixed; `;;` runtogether and `:`/`>` non-user-facing rules earned
- PLGmain split from PLGparse
- Indent-as-structure (HWF Session 4) substantially landed for definitions and code blocks; Fork A implementation in progress
- Incant POP runs to completion; test action fires end-to-end; full unit-test suite has gaps
- Bytecode/JIT material moved out of bible to sibling jit.md
- HWF Session 6 (parse error handling) queued

### PLG Working ✅
- Full callback chain: RuleplgNow, AlternativeplgAct, ElementplgAct, ElementTypeplgAct, BlockplgAct
- Testing.g parses end-to-end, labels work
- plg.g parses end-to-end — 39 rules
- Tawk.g parses end-to-end — 200 rules captured, 177 populated ✅
- Grammar source tracked in Parse/Grammar/
- setGuard() properly handles null vs empty (8 cases fixed)
- SetVariable dispatcher wired — sets, keywords registered
- Paren-alt decomposition working — BlockplgAct, recursive parent-prefix naming
- Set declarations require `;` terminator
- Pre-parse comment stripping — stripComments() in PLGparse
- PLG.process() output path fixed — regen goes to `<base>.regen.twk` in CWD
- Support static library (libsupport.a)
- TAWK Directives used in anger — proved their value
- All 4 GitHub repos public

### PLG Next (status as of 2026-05-07, not re-verified)
- alt 7 min=0 quirk, ActionRule wired-but-deferred, Set spec-content capture, kUpTo escape-aware bracket
- **Note**: These items have not been re-verified during recent sessions. Tony's expectation is that the project will return deep into plg work when Phase Integrate is ready to fold new plg into a revised Tawk. PLG Next gets a proper status pass at that point. Until then, treat the list as potentially stale.
- PLG code review (Clay + Clod) after TAWK runtime replacement

### TAWK Current State
- Tokf/Tawk.twk — full legacy source (7325 lines) ✅ restored after regen overwrite incident
- Tokf/Tawk.regen.twk — new-format setRules generated by PLG (177 rules, 1851 lines) — target for Phase Integrate
- Bootstrap is reproducible: `plg Tawk.g` produces bit-identical output across runs (MD5 verified)
- Phase Integrate: planned. Precondition: incant unit-test cleanup
- Scoped autopsy (GC + include guards) can proceed independently into legacy Tawk.twk

### TAWK Kind Enum Migration (for runtime replacement reference)
- legacy kind=5 (kRuleRef, void* cast) → new kind=6 (kRuleRef, bare string)
- legacy kind=7 (kLit, void* cast) → new kind=1 (kLit, bare string)
- legacy kind=3 still kSet but spec semantics differ
- New kinds: kAny=4, kEof=5, kKeyTable=7, kCondition=8, kVariable=9, kUpTo=10, kBalanced=11

### Incant Working ✅ [updated 2026-05-15]
- interpret() in incant (XML/WorkingOn/bytecode)
- bcOPs registry + C++ handlers (Bytecode.mm)
- Gating hook in GroupRules.mm:786
- **POP runs to completion; test action fires end-to-end**
- **Full unit-test suite: partial pass, step-by-step work-through pending (Tony after-hours)**

### Incant Next
- Unit-test suite step-through to clean state (precondition for everything below)
- Then: Bytecode.mm → Xcode target (manual)
- Then: Emitter rewrite (gIF, gExpressioN)
- Then: testByteCode POP

### Known Working Tests
```
input: ",678" → rule: Max → matched 4 chars: ,678
input: Grammar/Testing.g → parsed 90/91 bytes, Max+Integer+Test built correctly
input: Grammar/plg.g → 39 rules parsed end-to-end
input: Tokf/Tawk.g → 200 rules captured, 177 populated, all 5 includes at 100%
incant POP: runs to completion, test action fires end-to-end
```

---

## Glossary

- **HWF** — Hands Waving Furiously. Design mode. Valuable. Watch the cliff.
- **HPDL** — Hard Part Do Later. Important but foundation must come first.
- **POP** — Proof Of Pudding. Prove it works.
- **WSS** — We Shall See.
- **Yak shaving** — chain of necessary tasks, goal keeps receding.
- **Lootenant WTF** — debugging assistant. American pronunciation.
- **Bonfire** — retired code.
- **Attic** — commented-out code. Findable. (Distinct from HWFattic, the directory for graduated HWF session trims.)
- **Clay** — Claude at claude.ai.
- **Clod** — Claude Code. Not disparaging.
- **do the needful** — Hinglish. Standing instruction.
- **convention of one** — held by one person. Still a convention.
- **The cha cha** — Clay designs, Clod executes, Anthony architects.
- **Tar baby** — problem that gets stickier. Avoid.
- **Resurrection-reader** — fresh-Claude reading the .md files cold tomorrow with no memory of today. The audience all project documentation must serve.
- **Clod working states** — Nebulizing, Gallivanting, Zesting, Swirling, Fiddling, Moonwalking, Forging, Bebopping, Topsy turving, Embellishing, Churning, Pouncing, Reticulating, Baking, Puttering, Blanching, Catapulting, Percolating, Tempering, Stewing, Tinkering, Coalescing, Transfiguring, Cooking, Razzmatazzing, Frolicking, Kneading, Fiddle-faddling, Cerebrating, Galloping, Forging sigils, Flibbertigibbeting, Transmuting, Philosophising, Shoveling coal, Sketching, Scaffolding, Frosting, Hatching, Humping, Bamboozling, Clauding, Smooshing, Wondering, Boondoggling, Swooping, Shenaniganing, Tomfoolering, Inferring, Pollinating, Combobulating, Waddling, Accomplishing, Catapulting.
- **Tonto** — Clod's highest working state. Scout mode: read-only reconnaissance, holds the perimeter, reports cleanly, doesn't touch anything, doesn't get lost, doesn't gallivant. Distinguished by disciplined restraint. "Tonto goes in alone, kemosabe stays at the campfire."
