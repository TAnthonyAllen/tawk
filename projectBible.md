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
        Parse/          — PLG legacy source
        Parse/Revision/ — PLG new architecture (active codebase)
        Parse/Revision/Grammar/ — Grammar files: plg.g, plgRules.g, plg.act
        Parse/Tests/    — Test files + symlinks into Grammar/
        Tokf/           — TAWK source (tawk repo)
        Tokf/Tests/     — TAWK test sandbox (symlinks + Test.twk) — gitignored
        Tokf/Tawk.regen.twk — PLG-generated setRules in new format (gitignored artifact)
        Groups/         — Incant source (incant repo)
        Include/        — symlink → ~/data/support/Include
        Frame/          — symlink → ~/data/support/Frame
        Groups/Maps     — symlink → ~/data/support/Maps
    InProcess.xcworkspace — umbrella workspace containing all projects
    support/            — shared support classes (support repo)
        Frame/          — Buffer, DoubleLinkList, PLGset, BaseHash, Stak, Tape, StringRoutines
        Include/        — TAWK externals
        KeyTable/       — keyword lookup
        Maps/           — BitMAP, Segment
```

**Symlink architecture**: shared support classes live once in `~/data/support/`; the InProcess paths (`Include/`, `Frame/`, `Groups/Maps`, etc.) are symlinks pointing into the support repo. This keeps existing `.twk` `include` directives working unchanged while making the support repo the single source of truth. Edits to a shared class in either path land in the same file.

**APFS case-insensitive gotcha**: `plg.twk` and `PLG.twk` are the SAME file on macOS. Generated artifacts must live in a subdirectory. Grammar/ was created specifically to avoid this collision.

**PLG regen output**: `PLG.process()` writes to `<base>.regen.twk` in the directory where plg was invoked (CWD). When running on Tokf/Tawk.g from Tokf/Tests, output goes to Tokf/Tests/Tawk.regen.twk — NOT Tokf/Tawk.twk (the real source). plg's contract is "this file here, output here" — invocation-directory-relative, not source-file-resolved.

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
- **PLGparse** — base parser. Buffer-based input, cursor/eof, rules/setTable hashes, addTest(), getRule(), getSet(), parse(), divertInput/revertInput stack
- **PLGrule** — one grammar rule. Has alternatives (DoubleLinkList), guardSet, action callbacks (defer/immediate/fail)
- **Alternative** — one option within a rule. Has elements (DoubleLinkList), guardSet
- **Element** — one match unit. Kind (kLit/kChr/kSet/kAny/kEof/kRuleRef/kKeyTable/kCondition/kVariable/kUpTo/kBalanced), min/max, setRef/litText/ruleRef etc.
- **PLGitem** — match result. Buffer reference + offset, itemLength, toString()
- **PLG** — contains PLGparse via composition. setRules() and all plg-specific grammar/actions

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

## Grammar File Architecture

Grammar source in `Parse/Revision/Grammar/`:
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

## Paren-Alt Decomposition (BlockplgAct)

When a grammar rule contains inline `( A | B )` syntax, PLG decomposes it into a named helper rule during parse.

**Algorithm:**
1. `BlockplgAct` fires as a deferred callback when `Block` rule matches
2. Creates helper rule named `<parentRule>Block<N>` where N is a per-rule counter (resets per rule)
3. Recursive nesting: inner blocks use the helper as parent — `QuotedStringBlock0Block1`
4. All modifiers (label, min/max, noSkip, banged) transfer to the kRuleRef pointing at the helper

**Naming convention**: Recursive parent-prefix — `QuotedStringBlock0` → `QuotedStringBlock0Block1`. Counter resets per rule to prevent monotonic growth and name collisions.

**Important**: `( A | B )` cannot be stripped from PLG — TAWK's grammar uses it 5 times including nested forms. The decomposition machinery is required for TAWK ingestion.

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

## TAWK Runtime Replacement (Phase 2 Arc)

TAWK currently uses the legacy PLGparse API (kind=5/7 with void* casts, *TawkNow callbacks). New PLG generates the new API (kind=6 with bare strings). Bridging this gap is the Phase 2 arc.

**Phases:**
- A: Tawk.twk setRules() — splice Tawk.regen.twk body into legacy source
- B: Port ~50+ *TawkNow/*TawkAct callbacks — signatures change to (PLGparse state, PLGitem iTEM); children via iTEM.children["label"]
- C: First clean compile
- D: Tests/ sandbox verification vs ~/bin/tok
- E: Promotion

**Safety rules:**
- Tests/ directory in Tokf/ is mandatory sandbox — never touch Tokf/Tawk.twk before Tests/ proves it
- ~/bin/tok is the safety net — broken TAWK can't fix itself

**Independent of this arc:** Scoped TAWK autopsy (GC inheritance + include guards) goes directly into legacy Tokf/Tawk.twk.

---

## Resolved Issues Log

*For orientation — issues that seemed like blockers but were resolved.*

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

### Status
- Phase 0 (BDWGC) ✅
- Phase 1 (generateCode repurposed) ✅
- Phase 2 (bytecode emitter + interpreter) — in progress

### Key Design Decisions (settled)
- Bytecodes ARE GroupItems. No vregs — "a virtual register is just a GroupItem field."
- Stack-based dispatch via `interpret` sub-attribute on each op GroupItem
- `bcOPs` registry: bcBR, bcBRZ, bcCALL, bcRET — separate from user Operators
- `bytecodE` attribute name (Cap-on-last-letter convention)
- Instruction successor: implicit-next via for loop; branch by reassigning `grup` mid-loop

### interpret() — written in incant (XML/WorkingOn/bytecode)
```
interpret body; {
    for grup in body; members
        handler = grup.interpret;
        if !handler;
            print "interpret: no handler for" grup.taG:;
            return;
        result = handler(grup);
        if result;  grup = result;
    }
```
`for grup in body; members` — LoopRestrict `members` iterates members only. The for loop is a C++ while under the covers. Reassigning `grup` mid-loop redirects to branch target.

### Gating hook (GroupRules.mm:786) ✅
```cpp
GroupItem *bc = statement->getAttribute("bytecodE");
if ( bc ) {
    GroupItem *interpretField = GroupControl::groupController->locate("interpret");
    if ( interpretField )
        return ::runAction(bc, interpretField);
}
```
Falls through to gMethod when no bytecode — safe no-op until emitter produces bytecodE attributes.

### Remaining for testByteCode POP
1. `Bytecode.mm` → Xcode target (manual: drag into incantGUI)
2. Emitter rewrite: gIF, gExpressioN produce bytecodE attributes
3. End-to-end: `testByteCode` → `maximus = 26`

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

**Phase 2: TAWK Runtime Replacement (multi-session arc)**
- Phase A: Tawk.twk setRules() port — splice Tawk.regen.twk body
- Phase B: Port ~50+ *TawkNow/*TawkAct callbacks (signature change to PLGparse state, PLGitem iTEM)
- Phase C: First clean compile of new-runtime TAWK
- Phase D: Tests/ sandbox verification vs ~/bin/tok
- Phase E: Promotion

**Scoped TAWK autopsy (independent of runtime replacement)**
- GC inheritance fix — TAWK classes need to inherit from GC
- Include guard fix
- These go into LEGACY Tokf/Tawk.twk directly

**Phase 3: Incant bytecode/JIT**
- Emitter rewrite (gIF, gExpressioN)
- testByteCode POP
- Expand from there

**Longer term TAWK autopsy** — HPDL. Plan carefully — many .twk files need re-tokkifying.

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

## Current State (last updated: May 7 2026, session 6)

### PLG Working ✅
- Full callback chain: RuleplgNow, AlternativeplgAct, ElementplgAct, ElementTypeplgAct, BlockplgAct
- Testing.g parses end-to-end, labels work
- plg.g parses end-to-end — 39 rules
- Tawk.g parses end-to-end — 200 rules captured, 177 populated ✅
- Grammar source tracked in Parse/Revision/Grammar/
- setGuard() properly handles null vs empty (8 cases fixed)
- SetVariable dispatcher wired — sets, keywords registered
- Paren-alt decomposition working — BlockplgAct, recursive parent-prefix naming
- Set declarations require `;` terminator
- Pre-parse comment stripping — stripComments() in PLGparse
- PLG.process() output path fixed — regen goes to `<base>.regen.twk` in CWD (not source-file directory). Path-resolution simplification landed day 2 — process() works from invocation directory.
- Support static library (libsupport.a)
- TAWK Directives used in anger — proved their value
- All 4 GitHub repos public

### PLG Next
- Pending items: alt 7 min=0 quirk, ActionRule wired-but-deferred, Set spec-content capture, kUpTo escape-aware bracket
- PLG code review (Clay + Clod) after TAWK runtime replacement

### TAWK Current State
- Tokf/Tawk.twk — full legacy source (7325 lines) ✅ restored after regen overwrite incident
- Tokf/Tawk.regen.twk — new-format setRules generated by PLG (177 rules, 1851 lines) — target for Phase 2
- Bootstrap is reproducible: `plg Tawk.g` produces bit-identical output across runs (MD5 verified day 2)
- TAWK runtime replacement: Phase A-E planned, not yet started
- Scoped autopsy (GC + include guards) can proceed independently into legacy Tawk.twk

### TAWK Kind Enum Migration (for runtime replacement reference)
- legacy kind=5 (kRuleRef, void* cast) → new kind=6 (kRuleRef, bare string)
- legacy kind=7 (kLit, void* cast) → new kind=1 (kLit, bare string)
- legacy kind=3 still kSet but spec semantics differ
- New kinds: kAny=4, kEof=5, kKeyTable=7, kCondition=8, kVariable=9, kUpTo=10, kBalanced=11

### Incant Working ✅
- interpret() in incant (XML/WorkingOn/bytecode)
- bcOPs registry + C++ handlers (Bytecode.mm)
- Gating hook in GroupRules.mm:786

### Incant Next
- Bytecode.mm → Xcode target (manual)
- Emitter rewrite: gIF, gExpressioN
- testByteCode POP

### Known Working Tests
```
input: ",678" → rule: Max → matched 4 chars: ,678
input: Grammar/Testing.g → parsed 90/91 bytes, Max+Integer+Test built correctly
input: Grammar/plg.g → 39 rules parsed end-to-end
input: Tokf/Tawk.g → 200 rules captured, 177 populated, all 5 includes at 100%
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
- **Attic** — commented-out code. Findable.
- **Clay** — Claude at claude.ai.
- **Clod** — Claude Code. Not disparaging.
- **do the needful** — Hinglish. Standing instruction.
- **convention of one** — held by one person. Still a convention.
- **The cha cha** — Clay designs, Clod executes, Anthony architects.
- **Tar baby** — problem that gets stickier. Avoid.
- **Resurrection-reader** — fresh-Claude reading the .md files cold tomorrow with no memory of today. The audience all project documentation must serve.
- **Clod working states** — Nebulizing, Gallivanting, Zesting, Swirling, Fiddling, Moonwalking, Forging, Bebopping, Topsy turving, Embellishing, Churning, Pouncing, Reticulating, Baking, Puttering, Blanching, Catapulting, Percolating, Tempering, Stewing, Tinkering, Coalescing, Transfiguring, Cooking, Razzmatazzing, Frolicking, Kneading, Fiddle-faddling, Cerebrating, Galloping, Forging sigils, Flibbertigibbeting, Transmuting, Philosophising, Shoveling coal, Sketching, Scaffolding, Frosting, Hatching, Humping, Bamboozling, Clauding, Smooshing, Wondering, Boondoggling, Swooping, Shenaniganing, Tomfoolering, Inferring, Pollinating, Combobulating, Waddling, Accomplishing, Catapulting.
- **Tonto** — Clod's highest working state. Scout mode: read-only reconnaissance, holds the perimeter, reports cleanly, doesn't touch anything, doesn't get lost, doesn't gallivant. Distinguished by disciplined restraint. "Tonto goes in alone, kemosabe stays at the campfire."
