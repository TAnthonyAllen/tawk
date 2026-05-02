# The PLG/TAWK/Incant Project Bible
*A living document. Every sentence earns its place.*

---

## The Ecosystem in One Breath

**PLG** finds the pieces — a fast, declarative pattern matcher. Domain-specific, stateless, embeddable.  
**TAWK** shapes the expression — a source-to-source transpiler. Reads tawk syntax, outputs C++.  
**Incant** understands the meaning — reflexive, homoiconic, stack-aware, self-describing.

*PLG recognizes. TAWK transforms. Incant reasons.*

The three are one ecosystem. Working on any one inevitably involves the others.  
To refactor TAWK you first need to fix PLG. To finish Incant you need both.  
The design follows elegance; the classifications are just other nerds recognizing what was already there.

---

## Repositories

| Repo | URL | Status |
|------|-----|--------|
| PLG | https://github.com/TAnthonyAllen/plg | public ✅ |
| Incant | https://github.com/TAnthonyAllen/incant | public ✅ |
| Support | https://github.com/TAnthonyAllen/support | public ✅ |
| TAWK | https://github.com/TAnthonyAllen/tawk | public ✅ |

Each repo has a `CLAUDE.md` for Clod orientation and this bible (mirrored).

---

## Local Directory Map

```
/Users/anthony/Library/CloudStorage/Dropbox/data/
    InProcess/
        Parse/      — PLG source (plg repo)
        Tokf/       — TAWK source (tawk repo, pending)
        Groups/     — Incant source (incant repo)
        Include/    — symlink → ~/data/support/Include
        Frame/      — symlink → ~/data/support/Frame  
        Groups/Maps — symlink → ~/data/support/Maps
    support/        — shared support classes (support repo)
        Frame/      — Buffer, DoubleLinkList, PLGset, BaseHash, Stak, Tape, StringRoutines
        Include/    — TAWK externals (frame, maps, globals, plg.ext, groups.ext)
        KeyTable/   — keyword lookup
        Maps/       — BitMAP, Segment
```

---

## Language Classifications

| Project | Type | Key Properties |
|---------|------|----------------|
| PLG | DSL / pattern matcher | Declarative, stateless, fast, embeddable |
| TAWK | Transpiler | Source-to-source, metaprogramming, syntactic sugar for C++ |
| Incant | General purpose | Reflexive, homoiconic, stack-aware, generates to C++ |

**DSL** — Domain-Specific Language. Designed for one purpose. PLG only matches patterns.  
**Reflexive** — the language can examine and describe itself.  
**Homoiconic** — code and data have the same structure. A GroupItem field IS the rule that describes it.  
**Transpiler** — compiles to source code (C++) rather than machine code.  
**Declarative** — describes what to match, not how to match it.

---

## Architecture

### PLG Core Classes (new architecture)
- **PLGparse** — base parser. Buffer-based input, cursor/eof, rules/setTable hashes, divertInput/revertInput stack, addTest(), getRule(), getSet(), parse()
- **PLGrule** — one grammar rule. Has alternatives (DoubleLinkList), guardSet, action callbacks (defer/immediate/fail)
- **Alternative** — one option within a rule. Has elements (DoubleLinkList), guardSet
- **Element** — one match unit. Kind (kLit/kChr/kSet/kAny/kEof/kRuleRef/kKeyTable/kCondition/kVariable/kUpTo/kBalanced), min/max, setRef/litText/ruleRef etc.
- **PLGitem** — match result. Buffer reference + offset (not raw pointer), itemLength, toString()
- **PLG** — contains PLGparse via composition. setRules() and all plg-specific grammar/actions live here.

### Key Design Decisions
1. **Buffer-based input** — all input lives in Buffer objects. divertInput pushes current buffer, installs new one. No raw pointer arithmetic. No writable string problems.
2. **Safe iteration** — use `for link = list.first; link; link = link->next` pattern everywhere. Never use the default `next()` iterator in recursive contexts — it uses shared `entry` state that gets clobbered.
3. **Composition over inheritance** — PLG contains a PLGparse field rather than extending it (workaround for C++ incomplete type issues with TAWK-generated headers).
4. **toString() only** — PLGitem no longer null-terminates in place. toString() returns a malloc'd copy. string()/unString() retained for backward compatibility with support code.
5. **GC friendly** — tape allocator retired. New/delete or GC handles memory.
6. **Single exit point** — match methods use result variable + break pattern for debuggability.
7. **extern "C" bridge functions** — functions that bridge two types (neither owns the other) are `extern "C"` free functions. Pattern established by plg action callbacks (plgNow, plgAct). `foundIn(PLGset, PLGitem)` follows this pattern.
8. **Method ordering** — alphabetical by convention (Anthony's preference for Xcode navigability). Not a TAWK requirement.

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

### Input Diversion
```
divertInput(s):
    inputStack.push(buffer)
    buffer = new Buffer(s)
    cursor = buffer.start
    eof = buffer.end

revertInput():
    buffer = inputStack.pop()
    cursor = buffer.current
    eof = buffer.end
```

### addTest() — setRules() shorthand
```
addTest(kind, data, label, min, max, skipSet)
```
Creates an Element, sets all fields, adds to currentAlt. Reduces setRules() from 8 lines per element to 1.
`currentAlt` is a PLGparse field set before calling addTest(). `currentRule` similarly.

---

## The Bootstrapping Problem

PLG is supposed to generate setRules() but needs setRules() to parse plg.g to generate setRules().

**Current bootstrap sequence:**
1. Old PLG binary runs on `plg.g` → generates `plg.twk` (old addTest format)
2. `python3 translateSetRules.py plg.twk new_setRules.twk` → translates to new Alternative/Element format
3. Paste `setRules()` from `new_setRules.twk` into `PLG.twk`
4. Run `tawk PLG.twk` → `PLG.C`
5. Compile and test

**Long term goal**: New PLG parses `plg.g` and generates its own `setRules()`. Self-hosting.

**Same problem exists for TAWK**: building tawk-from-source requires a working tawk binary. Currently solved by `~/bin/tok`. Any clean-checkout build story needs to address this.

---

## TAWK Known Issues (autopsy table)

1. **Empty comment lines** (`//`) reset field resolution context — causes `new` to fail type inference and field references to be lost. Fix: context tracking should be comment-blind.
2. **Implicit field resolution** is complex machinery. Incant solves this elegantly with `lastREF`/`@context`. TAWK refactor: replace implicit resolution with explicit `@field` context markers.
3. **No include guards** generated in .h files. Must add manually for inherited classes.
4. **`new` type inference** inconsistent — `alt = new` sometimes fails to resolve to `new Alternative()`. Workaround: explicit `alt = new Alternative()`. Known quirk, not worth fixing until TAWK refactor.
5. **No `#include` in .h files** — inherited classes need parent .h included but TAWK never generates includes in .h files. Must add manually after every re-tawk.
6. **Unused field warning** — TAWK silently drops unused declared fields instead of warning. Fix: emit warning, keep field.
7. **`kSet` macro conflict** — `kSet(button)` is a test macro, not an assignable value. Kind assignment requires numeric literal. Fix: separate test macros from enum values.
8. **TAWK error propagation** — when TAWK encounters unresolved references, it embeds the error INTO the generated C++ file as invalid code. The C++ compiler (Xcode) then flags it. Workflow: run tawk → compile in Xcode → fix tawk errors flagged by Xcode → repeat. Annoying but effective.

---

## Incant / PLG Convergence

- `match()` — atomic recognition. Did this pattern occur here?
- `parse()` — semantic processing. What does it mean that it occurred?
- In PLG these are separate. In Incant they collapse because the data structure IS the grammar IS the result.
- **The bridge**: PLGitem is a lightweight match result. GroupItem wraps PLGitem when semantics are needed. Promotion is lazy — only when incant needs to reason about the result.
- **Input diversion**: same concept in both. PLG: push/pop Buffer. Incant: push/pop GroupItem containing buffer. Same contract, appropriate implementation for each.
- **The north star**: PLG written in Incant, parsing itself. Not today. But the architecture points there.

---

## generateRules() / setRules() relationship

`generateRules()` is PLG's primitive bytecode compiler — it promotes interpreted grammar structures to compiled C++.  
Incant's bytecode work is the same idea at higher abstraction.  
`parse()` orchestrates. `match()` is atomic. `generateRules()` compiles.

---

## Support Library Architecture

Support classes are shared across PLG, TAWK, and Incant. They live in the support repo.

**Dependency rule**: support classes must not depend on PLG/TAWK/Incant classes. If a dependency creeps in, extract a bridge function (`extern "C"`) rather than adding an include.

**Known bridge**: `foundIn(PLGset*, PLGitem*)` — lives in PLGparse.C as `extern "C"`. PLGset needs to check if a PLGitem matches a set, but PLGset must not include PLGitem.h.

**Target architecture**: support as a static library (support.a), linked into each tool. Currently support files are compiled inline — static library refactor in progress.

**PLGset note**: PLGset was previously in Parse (PLG source). Moved to support/Frame. It's general-purpose pattern-set machinery used by all three projects.

---

## Working Relationship

**Anthony** — architect, domain expert, final authority on design decisions. Coding as thinking out loud.  
**Clay** (Claude at claude.ai) — design, reasoning, architecture, HWF navigation, code review.  
**Clod** (Claude Code) — execution, file edits, GitHub maintenance, build verification.

**Standing permissions**: Clod may change any code in source directories without asking. Ask before GitHub pushes.  
**HWF protocol**: When design goes near a cliff, flag it. The face plants teach as much as the successes.  
**Convention of one**: tawk not tok in source (Tony's awk). Conventions are conventions even with one member.  
**The cha cha**: Clay designs, Clod executes, Anthony architects. Each dancer has a distinct role.

---

## Current State (last updated: session 3)

### Working
- PLGparse, PLGrule, Alternative, Element, PLGitem compiling and running
- Buffer-based input with divertInput/revertInput
- matchSet, matchLit, matchRuleRef working correctly
- applyRepetition working
- Safe DoubleLinkList iteration established
- addTest() implemented in PLGparse
- PLGset moved to support repo
- GitHub repos: plg, incant, support, tawk all public ✅
- Xcode project rebuilt via xcodegen/project.yml — reproducible
- CLAUDE.md in plg and support repos
- `matched 4 chars: ,678` — known-good test passing

### In Progress
- Support static library refactor (foundIn dependency cycle being resolved)
- setRules() shrinkage using addTest()
- Xcode workspace (Shape B) — plg + support + future tawk/incant

### Next Steps
1. Resolve foundIn dependency cycle → complete static library refactor
2. Shrink setRules() using addTest()
3. Feed testing.g through parser instead of hardcoded setRules()
4. Expand to plg.g — bootstrap new PLG
5. Xcode workspace wiring
6. TAWK on the autopsy table (after PLG bootstraps)
7. Incant JIT (longer term)

### Known Working Test
```
input: ",678"
rule:  Max
result: matched 4 chars: ,678
```

---

## Glossary

- **HWF** — Hands Waving Furiously. Design mode with reduced constraint on anchor to reality. Valuable. Watch the cliff.
- **Lootenant WTF** — debugging assistant. Finds culprits. Earns commendations. (British: Leftenant. American: Lootenant. We go American.)
- **Bonfire** — retired code. Gone but not mourned.
- **Attic** — commented-out code. Not active, not deleted, findable.
- **Clay** — Claude at claude.ai. Design, reasoning, architecture. (Claude → Clay. Shorter.)
- **Clod** — Claude Code. Execution, file ops, GitHub. Not disparaging. Just shorter.
- **do the needful** — Hinglish. Do what needs doing. Standing instruction to Clod.
- **eRocka** — modern eureka. Reserved for significant breakthroughs.
- **convention of one** — a convention held by exactly one person. Still a convention.
- **DSL** — Domain-Specific Language. A language designed for one specific purpose.
- **YAML** — Yet Another Markup Language. Human-friendly config format. Used for xcodegen project specs.
- **Nebulizing** — what Clod does when given a complex task. Activity is happening. Results pending.
- **Gallivanting** — Clod charging off in all directions on a complex task. Usually ends somewhere useful.
- **Zesting** — adding citrus flavor to the codebase. Meaning unclear but output is always fresh. 🍋
- **The cha cha** — the three-way workflow: Clay designs, Clod executes, Anthony architects. An elegant dance.
- **Tar baby** — a problem that gets stickier the more you engage with it. Avoid when possible.
- **extern "C" bridge function** — a free function using C linkage (no name mangling) that bridges two types. Pattern for cross-type utilities that neither type should own.
