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
        Groups/         — Incant source (incant repo)
        Include/        — symlink → ~/data/support/Include
        Frame/          — symlink → ~/data/support/Frame
        Groups/Maps     — symlink → ~/data/support/Maps
    support/            — shared support classes (support repo)
        Frame/          — Buffer, DoubleLinkList, PLGset, BaseHash, Stak, Tape, StringRoutines
        Include/        — TAWK externals
        KeyTable/       — keyword lookup
        Maps/           — BitMAP, Segment
```

**APFS case-insensitive gotcha**: `plg.twk` and `PLG.twk` are the SAME file on macOS. Generated artifacts must live in a subdirectory. Grammar/ was created specifically to avoid this collision.

---

## Language Classifications

| Project | Type | Key Properties |
|---------|------|----------------|
| PLG | DSL / pattern matcher | Declarative, stateless, fast, embeddable |
| TAWK | Transpiler | Source-to-source, metaprogramming, syntactic sugar for C++ |
| Incant | General purpose | Reflexive, homoiconic, stack-aware, generates to C++ |

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
2. **Safe iteration** — `for (link = list->first; link; link = link->next)` — never add manual advance inside body (TAWK's for loop already advances — double-advance = SIGSEGV).
3. **Composition over inheritance** — PLG contains PLGparse field (workaround for TAWK header issues).
4. **toString() only** — PLGitem no longer null-terminates in place.
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

**Self-host status**: plg.g parses end-to-end (37 rules). Generator emits correct addTest() format (76% smaller). Round-trip chain stalls at bare-include over-matching — next debug cycle, use plgDirectives.

**Same problem exists for TAWK**: solved by `~/bin/tok`.

---

## Grammar File Architecture

Grammar source in `Parse/Revision/Grammar/`:
- `plg.g` — rule structure. 4 changes from original: plgRules.g include, action.g excluded, bare-include support, Start rule fixed to `Header* '%%' Body+ '%%'? Trailer?`
- `plgRules.g` — shared foundational rules (Comment+CommentBody, Integer, Name, Label etc.)
- `plg.act` — action code
- `action.g` — DEFERRED pending Action-blocks feature

**Action blocks design** (approved, TODO):
```
Action actionName { TAWK body } ;
```
Generates: `void actionName(PLGparse *state, PLGitem *item) { body }`
Goal: grammar files self-contained, no separate .act/.rtn files needed.

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

## Incant Bytecode (Phase 2 — in progress)

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

Claude is a GroupItem — `isCLAUDE` alongside `isSTRING`, `isNUMBER`, `isGROUP`. The AI is not a tool called from incant — it IS a field in incant.

Go-style channel messaging — steal Go's goroutine/channel pattern. HPDL. Ken Thompson approved.

ZFS-flavored storage — copy-on-write, snapshots as GroupItem operations. HPDL.

The JIT is the enabling technology. Without JIT, incant is an interpreter. With JIT, incant ships.

---

## Working Relationship

**Anthony (Haps)** — architect, domain expert, final authority.  
**Clay** (Claude at claude.ai) — design, reasoning, architecture, HWF navigation.  
**Clod** (Claude Code) — execution, file edits, GitHub, build verification.

**Standing permissions**: Clod changes any code in source directories without asking. Ask before GitHub pushes. No option menus — do the needful.  
**Clod protocols**: "got it" when message lands. "ready" when done. PLG:/Incant: labels when parallel tracks.  
**End-of-session ritual**: Clay drafts bible + TODO, Clod pushes to all 4 repos. Before every Goodnight Gracie.

---

## Current State (last updated: May 4-5 2026)

### PLG Working ✅
- Full callback chain: RuleplgNow, AlternativeplgAct, ElementplgAct, ElementTypeplgAct
- Testing.g parses end-to-end, labels work
- plg.g parses end-to-end — 37 rules
- Grammar source tracked in Parse/Revision/Grammar/
- setGuard() properly handles null vs empty (8 cases)
- Support static library (libsupport.a)
- All 4 GitHub repos public

### PLG Next
- Self-host: bare-include over-matching (use plgDirectives)
- Action blocks feature
- Grammar reorganization

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
input: Grammar/plg.g → 37 rules parsed end-to-end
```

---

## Glossary

- **HWF** — Hands Waving Furiously. Design mode. Valuable. Watch the cliff.
- **HPDL** — Hard Part Do Later. Important but foundation must come first.
- **POP** — Proof Of Pudding. Prove it works.
- **WSS** — We Shall See.
- **eRocka** — modern eureka.
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
- **Clod working states** — Nebulizing, Gallivanting, Zesting, Swirling, Fiddling, Moonwalking, Forging, Bebopping, Topsy turving, Embellishing, Churning, Pouncing, Reticulating, Baking, Puttering, Blanching, Catapulting, Percolating, Tempering, Stewing, Tinkering, Coalescing, Transfiguring, Cooking, Razzmatazzing, Frolicking, Kneading, Fiddle-faddling, Cerebrating, Galloping, Forging sigils, Flibbertigibbeting, Transmuting, Philosophising, Shoveling coal, Sketching, Scaffolding, Frosting, Hatching, Humping, Bamboozling, Clauding, Smooshing, Wondering, Boondoggling, Swooping, Shenaniganing, Tomfoolering, Inferring, Pollinating, Combobulating.
