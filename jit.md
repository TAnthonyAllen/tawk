# Incant Bytecode / JIT
*Design and status for the incant bytecode interpreter and eventual JIT.*
*Sibling document to projectBible.md. Created 2026-05-15 from a spin-off out of the bible.*

---

## Why this document exists

The bible's job is to orient resurrection-readers on wake-up. Bytecode/JIT material accumulated in the bible to the point where it bulked up the wake-up read with forward-looking design detail that doesn't help on first orientation. This document holds that material so the bible can stay focused.

When this work is the active arc, this is the document to read. When it isn't, the bible's status summary is enough.

---

## Status

- **Phase 0 (BDWGC)** ✅ Complete
- **Phase 1 (generateCode repurposed)** ✅ Complete
- **Phase 2 (bytecode emitter + interpreter)** — In progress

**Precondition for Clod-driven emitter work**: incant unit-test suite passing clean. Pointing Clod at emitter rewrites while the interpreter has gaps means a failing testByteCode run can't be attributed cleanly to emitter vs interpreter — and Clod cycles would burn on phantom bugs.

---

## Key Design Decisions (settled)

- **Bytecodes ARE GroupItems.** No vregs — "a virtual register is just a GroupItem field."
- **Stack-based dispatch** via `interpret` sub-attribute on each op GroupItem.
- **`bcOPs` registry**: bcBR, bcBRZ, bcCALL, bcRET — separate from user Operators.
- **`bytecodE` attribute name** (Cap-on-last-letter convention).
- **Instruction successor**: implicit-next via for loop; branch by reassigning `grup` mid-loop.

The "bytecodes are GroupItems" decision is the load-bearing one. It collapses what would otherwise be a separate intermediate representation into the same machinery incant already uses for everything else — homoiconic at the bytecode layer, not just the source layer. The interpreter is written in incant itself (see below), exercising the language on its own runtime.

---

## interpret() — written in incant

Located in `XML/WorkingOn/bytecode` in the incant repo:

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

**`for grup in body; members`** — LoopRestrict `members` iterates members only. The `for` loop is a C++ while under the covers. Reassigning `grup` mid-loop redirects to a branch target.

**Pattern**: each op GroupItem carries its own `interpret` handler as a sub-attribute. Dispatch looks up the handler, fires it on the op, and either lets the loop advance to the next op (implicit-next) or has the handler return a different op to jump to (branch).

---

## Gating hook (GroupRules.mm:786) ✅

```cpp
GroupItem *bc = statement->getAttribute("bytecodE");
if ( bc ) {
    GroupItem *interpretField = GroupControl::groupController->locate("interpret");
    if ( interpretField )
        return ::runAction(bc, interpretField);
}
```

Falls through to gMethod when no bytecode — safe no-op until the emitter produces `bytecodE` attributes. This means the gating hook can land before the emitter is ready without breaking anything.

---

## Remaining for testByteCode POP

1. **`Bytecode.mm` → Xcode target** (manual: drag into incantGUI target)
2. **Emitter rewrite**: gIF, gExpressioN produce `bytecodE` attributes
3. **End-to-end**: `testByteCode` → `maximus = 26`

Once testByteCode runs and produces the expected value, the bytecode interpreter is POP'd and the JIT arc can be planned in earnest.

---

## Incant Dispatch Idiom (load-bearing)

Two steps — never chain:
```
handler = field.attribute;    // get the attribute
handler(argument);            // call its method
```
One method per field by design. Sub-attribute pattern for second invokable behavior. The bytecode interpreter relies on this idiom in `handler = grup.interpret` followed by `handler(grup)` — same shape, used at the runtime layer.

---

## The Long Game

The JIT is the enabling technology. Without JIT, incant is an interpreter. With JIT, incant ships.

The bytecode work is the precondition for JIT: the bytecode IR is what the JIT will compile. The "bytecodes are GroupItems" decision means the JIT operates over the same data structures the rest of incant uses, which keeps the homoiconic story intact at every layer.

The Phase 3 arc — emitter rewrite, testByteCode POP, expansion from there — is the path from "interpreter that runs" to "interpreter ready for JIT."

---

## Cross-references

- Bible Architecture section — for how PLG and the rest of the runtime relate
- Bible Incant Grammar section — for the source-language conventions
- HWF Session 4 — for the indent-as-structure work that's reshaping incant grammar in parallel
- TODO — for current Phase 3 task sequencing
