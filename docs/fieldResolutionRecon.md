# TAWK Field Resolution Recon

A guided tour of the field-resolution layer in TAWK — how the transpiler answers
"what symbol does this name refer to?" Written for cold reading by future-Tony
or future-Clay before any refactor work begins.

This document covers the semantic resolution layer. It does NOT cover parsing
(PLG's job) or code generation (FormatC's job, mostly), except where those layers
*call into* resolution. The recon is bounded to Instance.twk, InstanceTable.twk,
Symbol.twk, SymbolType.twk, with peripheral notes on Block.twk, Expression.twk,
and the resolution call sites in Tawk.twk that hit InstanceTable.

---

## 1. The Big Picture

Four classes collaborate. Their roles are distinct but the boundaries leak in
several places (see §5 Tar Babies).

```
              ┌────────────────────────────────────────────┐
              │  Symbol         "I am the declaration"     │
              │  - name, type, parameters, parentClass     │
              │  - isAlias→source, isExtension, isMethod   │
              └─────────────────────┬──────────────────────┘
                                    │ described by
              ┌─────────────────────▼──────────────────────┐
              │  Instance       "I am a reference"         │
              │  - symbol|type|express|statement|block     │
              │  - cast, parent, parameters                │
              │  - all the isFoo boolean flags             │
              └─────────────────────┬──────────────────────┘
                                    │ stacked / indexed in
              ┌─────────────────────▼──────────────────────┐
              │  InstanceTable  "I am the current scope"   │
              │  - instances (stack, keyed by name)        │
              │  - globalFields (BaseHash)                 │
              │  - scopeStack, presentClass                │
              └─────────────────────┬──────────────────────┘
                                    │ delegates type-side lookups to
              ┌─────────────────────▼──────────────────────┐
              │  SymbolType     "I am a class"             │
              │  - components, methods, componentFields    │
              │  - componentTypes (descendent chain)       │
              │  - overloads, parent (inheritance)         │
              └────────────────────────────────────────────┘
```

**One-line summary of each:**
- **Symbol** — a declaration: a name with a type, possibly a method, possibly an alias.
- **Instance** — a polymorphic node in the AST. Wraps one of {symbol, type, express, statement, block} plus context (cast, parent, parameters, flags).
- **InstanceTable** — the scope stack. Holds instances currently in scope and offers `find(name)` to look one up.
- **SymbolType** — a class or primitive. Owns its components (fields), methods, componentFields (precomputed ancestor descent), and operator overloads.

---

## 2. Entry Points

`InstanceTable.find()`, `findInstance()`, `findSymbol()`, and `findMethod()` are
the public entry points for name-to-instance resolution. They live on the
singleton `currentSymbols` held by Tawk.

**Where `currentSymbols` is created and used** (Tawk.twk):

| Line  | Code | Purpose |
|-------|------|---------|
| 384   | `InstanceTable currentSymbols;` | field on Tawk |
| 436   | `currentSymbols = new;` | initialized once |
| 466   | `currentSymbols.getConverter(subjectType, objectType)` | type conversion resolution |
| 522   | `currentSymbols.find("concat")` | direct lookup by name |
| 581   | `currentSymbols.findMethod(instance)` | method-by-signature lookup |
| 624   | `currentSymbols.find(method.methodName)` | by mangled name |
| 844 / 853 / 858 / 872 | `currentSymbols.find("printf"|"sprintf"|"asprintf")` | format-routine resolution for `print` |
| 922   | `currentSymbols.presentClass = t` | sets the "currently compiling" class |
| 2972  | `currentSymbols.pop("Block end")` | exit scope on block close |
| 2999  | `currentSymbols.push("Block start")` | enter scope on block open |
| 3004  | `currentMethod.pushParameters(currentSymbols)` | seed scope with method args |
| 3024  | `currentSymbols.push(currentClass.name)` | enter class scope |
| 3653  | `currentSymbols.findMethod(current)` | (in alias / extension resolution) |
| 4072  | `currentSymbols.findField(text)` | (in declarations) |
| 4726  | `currentSymbols.findMethod(current)` | (in call-site resolution) |
| 6398  | `currentClass.findField(text)` | direct SymbolType lookup |

**Where Symbol/SymbolType lookups bypass InstanceTable entirely**:
Many call sites in Tawk.twk go straight to `currentType.findField(text)` or
`poType.findField(text)` (Tawk.twk:652, 670, 6398) — these are the "I already
know which type to look in" path. InstanceTable handles the "I don't know which
type" path.

---

## 3. End-to-End Resolution Flow

Given a name token from the parser, here is what happens. (Reading
InstanceTable.twk:156–300 alongside this.)

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          InstanceTable.find(name)                       │
│                                                                         │
│  1. Direct hit in current scope stack:                                  │
│     instance = instances.get(name)                                      │
│                                                                         │
│  2. If miss:  ───────────────►   findInstance(name)                     │
│                                  (see expanded box below)               │
│                                                                         │
│  3. If hit: make a *copy*, set isDeclaration=false, attach              │
│     foundAncestor as parent.                                            │
│                                                                         │
│  4. Clear foundAncestor.                                                │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│                  InstanceTable.findInstance(name)                       │
│                                                                         │
│  resetIsFlagged()                                                       │
│  foundAncestor = null                                                   │
│                                                                         │
│  PASS A — Direct hit in stack:                                          │
│    if instances[name]  ──►  return                                      │
│                                                                         │
│  PASS B — Walk stack LIFO, descend into each scope's TYPE:              │
│    for instance in instances.prior():                                   │
│        if instance.isMethod: match on methodName, no descent            │
│        if instance.howDirect() > 1: skip (indirection too deep)         │
│        type = instance.getType()                                        │
│        if type already flagged or atomic: skip                          │
│        type.isFlagged = true                                            │
│        foundAncestor = instance                                         │
│        field = type.findFieldInstance(name)  ───┐                       │
│        searchForField   ◄────── (#macro)        │                       │
│            tracks lowest level + sets           ▼                       │
│            last, lastParent          ┌──────────────────────┐           │
│                                      │ SymbolType-side recursion (§3a)│ │
│                                      └──────────────────────┘           │
│                                                                         │
│  PASS C — presentClass and its components:                              │
│    type = presentClass                                                  │
│    field = type.findFieldInstance(name)                                 │
│    searchForField  (same tracking)                                      │
│                                                                         │
│  PASS D — globals:                                                      │
│    if globalFields[name]:  return  (direct hit)                         │
│    if !last (nothing found yet):                                        │
│        walk globalFields.prior() similarly to PASS B                    │
│                                                                         │
│  foundAncestor = lastParent                                             │
│  return last (lowest-level match across all passes)                     │
└─────────────────────────────────────────────────────────────────────────┘
```

### 3a. SymbolType-side recursion

`type.findFieldInstance(name)` does the type-side descent. It calls
`fillComponentFields(name)`, which:

```
fillComponentFields(text)               SymbolType.twk:516
├── isFilled = true
├── if !hasDescendentTypes:
│      addAncestorTypes()              SymbolType.twk:232
│         walks parent chain, copies parent's components and methods
│         into componentFields, then descends *componentTypes*
│         building a chain of Instance.parent pointers
├── if componentFields[text] already cached:
│      return cached (may be nullInstance sentinel)
├── walk componentTypes (LIFO):
│      for ancestor in componentTypes.prior():
│          type = ancestor.getType()
│          component = type.componentFields[text]
│          if missing and type not filled:
│              recurse: type.fillComponentFields(text)
│          if found: track lowest (component.level + ancestor.level)
│              build chain: descendent.setParent(ancestor)
│              cache in componentFields[text]
│              if level == 2: break (closest direct ancestor wins)
└── if no match: cache nullInstance (negative cache)
```

The level math (`component.level + ancestor.level`) is the "closest match wins"
mechanism. Combined with `findInstance`'s `lowest`/`last` tracking, the overall
resolution prefers the field that's reachable by the shortest parent-chain walk.

### 3b. Method resolution variant

`findMethod(source)` (InstanceTable.twk:189) differs:

1. Mangles `source` (or uses `source.symbol.methodName` if already a method).
2. If `source.prefix` and isMethod, try `types.get(prefix).findMethodInstance(source)`
   — this is the constructor-call path.
3. Otherwise `findInstance(name)` (same machinery as above) with the mangled name.
4. SymbolType.findMethod (line 641) does a fallback: if mangle-lookup misses,
   walks `methods.next(source.prefix)` and uses `Symbol.matchMethod(source)` for
   fuzzy match (handles ellipsis, void*, idType↔OC).

### 3c. Scope push/pop

`push(text)` saves `count` on `scopeStack`, increments `scope`.
`pop(text)` restores `count` and pops `instances` back down to it.

This is the bracket-by-bracket scope machinery. Called by Tawk.twk:2999 on block
start and 2972 on block end. `Symbol.pushParameters(table)` (Symbol.twk:543)
seeds the new scope with a method's parameters.

---

## 4. Data Structures Consulted

### Instance (Instance.twk, 1298 lines)

Polymorphic AST wrapper. Holds at most one of:
- `block` (Block) — a brace-bounded sequence of statements
- `express` (Expression) — a subject-verb-object structure
- `statement` (Statement) — a higher-level statement node
- `symbol` (Symbol) — a reference to a declared symbol
- `type` (SymbolType) — a type reference (e.g., for casts)

Plus context:
- `cast` (Instance) — wrapped cast target type
- `format` (Instance) — printf format string
- `parent` (Instance) — *parent in the resolution chain*, not AST parent
- `parameters` (DoubleLinkList) — method args, or `[]` indices for array refs
- 30+ boolean flags (isMethod, isAlias, isCast, isDeclaration, …)

The **`parent` chain** is the key field-resolution navigation. When
`InstanceTable.find` succeeds, it sets `result.parent = foundAncestor` —
encoding the path `foundAncestor.result`, i.e., "you'll get to this field by
dereferencing foundAncestor first."

Methods most relevant to resolution:
- `getType()` (line 797) — type-resolution dispatch over cast/type/symbol/express/statement
- `getSymbol()` (line 784) — recurse through subject to find a Symbol
- `getSubject()` (line 772) — descend statement/express to leaf
- `checkOverload()` (line 326) — operator overload resolution; long (170+ lines), with goto labels
- `checkSymbol()` (line 505) — alias-chain walk + C-method `tHIS` parameter insertion
- `findGetterOrSetter(name)` (line 585) — walks parent chain looking for a named method
- `sourceType()` (line 1252) — like getType but ignores alias type, drilling to underlying
- `mangle()` (line 963) — builds a method name from parameter types (input to findMethod)

### InstanceTable (InstanceTable.twk, 404 lines)

The scope stack and globals.

| Field | Purpose |
|-------|---------|
| `instances` (DoubleLinkList, hasKeys=true) | scope stack, keyed by symbol name |
| `globalFields` (BaseHash) | global field instances |
| `scopeStack` (Stak) | push/pop boundary marks (saved `count` values) |
| `scope` (int) | nesting level counter |
| `presentClass` (SymbolType) | the class currently being compiled |
| `foundAncestor` (Instance) | **mutated during find**, consumed by caller |
| `count` (long) | total instances on stack (the pop-down mark) |

Public lookup methods: `find`, `findInstance`, `findMethod`, `findSymbol`,
`findGlobalMethod`, `getConverter`. Maintenance: `add`, `addGlobalField`,
`push`, `pop`.

**Methods get two table entries**: under `symbol.name` AND under
`symbol.gitMethodName()` (mangled form) — see `add()` line 39–41. The comment
on line 25–27 notes this as a known smell: "may want to maintain a separate
namespace??"

### Symbol (Symbol.twk, 615 lines)

A declaration. Owns:
- identity (`name`, `methodName`, `symbolIndex`)
- type pointers (`type`, `structType`, `parentClass`)
- alias chain (`isAlias`, `source`)
- method machinery (`parameters`, `block`, `isMethod`, `isExtension`)
- property machinery (`getter`, `setter`)
- 30+ booleans (isStatic, isConst, isHidden, isOCfield, …)

Resolution-relevant methods:
- `gitMethodName()` (line 329) — mangled methodName getter (calls `mangle()` lazily)
- `mangle()` (line 415) — builds `name(type1,type2*,…)` from parameters
- `matchMethod(Instance target)` (line 480) — fuzzy match against an Instance; handles ellipsis (nullType), void*, idType↔OC interop
- `pushParameters(InstanceTable)` (line 543) — seed a fresh scope with this method's params
- `extendType()` (line 193) — register this method as an extension of its first parameter's type
- `makeAlias(name)` (line 357) — clone-with-rename for aliasing
- `setRefer()` (line 580) — mark this symbol (and its source/type/parentClass/parameters) as referenced

### SymbolType (SymbolType.twk, 1186 lines)

A class. Owns components, methods, ancestor relationships.

| Field | Purpose |
|-------|---------|
| `components` (BaseHash) | direct member symbols by name |
| `methods` (BaseHash) | direct method symbols by name AND by methodName |
| `componentFields` (BaseHash) | cache of resolved field-instances, including ancestor descent |
| `componentTypes` (DoubleLinkList) | descendent-type instances with parent chain (built by addAncestorTypes) |
| `descendentTypes` (DoubleLinkList) | child classes (set by setParent) |
| `overloads` (DoubleLinkList) | op → method name map |
| `parent` (SymbolType) | inheritance parent |

Resolution-relevant methods:
- `findField(text)` (line 611) — components ∪ methods, NO descent
- `findFieldInstance(text)` (line 628) — wraps `fillComponentFields` (full descent + caching)
- `fillComponentFields(text)` (line 516) — heavy ancestor-chain walk, with nullInstance sentinel for negative cache
- `addAncestorTypes()` (line 232) — populates componentTypes/componentFields by walking parent + descending into componentTypes
- `getLocal(name)` (line 740) — components/methods only, then hidden-embedded-struct descent, then parent chain
- `getMethod(name)` (line 767) — methods only, walks parent chain
- `findMethod(source)` (line 641) — by mangle, fallback to fuzzy match via Symbol.matchMethod
- `findMethodInstance(source)` (line 667) — like findMethod but returns Instance
- `findAliasTarget(text)` (line 582) — one-level-deep search for getter/setter wiring
- `getConverter(target, source)` (line 711) — type-conversion method lookup
- `overloaded(op)` (line 975) — op-to-method-name lookup
- `matches(type)` (line 949) — type equivalence with Char/Number families collapsing

The **two flagging mechanisms** are worth noting:
- `isFlagged` (bool, on SymbolType) — used by findAliasTarget and findInstance to avoid revisiting
- `isFilled` (bool, on SymbolType) — used by fillComponentFields to avoid re-descending

Both rely on companion reset functions (`resetIsFlagged`, `resetIsFilled` —
defined in Types.twk) and assume single-threaded sequential resolution.

---

## 5. Tar Babies

The places where the abstraction leaks or the code is doing more than its name
suggests. Listed in rough order of how much they would bite a refactor.

### 5.1 `foundAncestor` is shared mutable state on InstanceTable

InstanceTable.twk:241, 248, 262, 270, 280, 292, 298, 170.

`foundAncestor` is a field on InstanceTable. `findInstance` mutates it during
its walk; `find` consumes it after the call returns to attach as parent.

This is a single-threaded assumption baked in. Any nested resolution call
(e.g., during a recursive type walk) would clobber the outer call's
`foundAncestor`. The code today probably gets away with it because resolution
isn't reentrant in the call paths exercised — but the contract is implicit and
fragile.

### 5.2 The `#searchForField-` macro depends on local-variable closure

InstanceTable.twk:214–227.

```
#searchForField-
if field && (symbol = field.getSymbol())
    {
    if symbol.isStatic return field;
    currentLevel = field.level;
    if foundAncestor currentLevel++;
    if !lowest || currentLevel < lowest
        {
        lowest  = field.level;
        last    = field;
        lastParent  = foundAncestor;
        if foundAncestor lowest++;
        }
    }-
```

This is a TAWK include macro — invoked as `searchForField;` at three sites in
findInstance. It assumes the inlining context has `field`, `symbol`,
`currentLevel`, `lowest`, `last`, `lastParent`, `foundAncestor` all in scope.
The same macro doesn't work in any other context. The level-arithmetic is also
subtle: `lowest = field.level` then `if foundAncestor lowest++` — the lowest
mark is "field level plus 1 if found through an ancestor."

If/when this gets cleaned up, the macro should probably become a method that
returns a struct (field, level, parent), and the caller picks the lowest. The
shared-mutable-locals pattern is also why the macro isn't a method today — it's
mutating four loop variables.

### 5.3 `fillComponentFields` caches `nullInstance` for negative lookups — no invalidation story

SymbolType.twk:564–565, 524.

```
if !instance
    componentFields[text] = nullInstance;
```

This is great for repeated misses (avoids re-descending the ancestor chain),
but if a type later acquires a new member (e.g., extension method added via
`Symbol.extendType()`), the cached null persists and the new member is
invisible to subsequent lookups.

In current usage this may not bite — types likely complete their definition
before resolution starts. But the order-of-operations dependency is undocumented
and worth confirming before any refactor that interleaves type-extension and
resolution.

### 5.4 Possible bug in `findSymbol`

InstanceTable.twk:305–326.

```
Symbol findSymbol(String name)
{
Instance	field;
Symbol      symbol;          // ← declared, never assigned in branches below
    if field = instances[name]
        if field.symbol return symbol;        // returns the local `symbol` (null)
    symbol = presentClass.findField(name);    // ← here it IS assigned
        if symbol return symbol;
    if globalFields
        if field = globalFields[name]
            if field.symbol return symbol;    // returns the local `symbol` (null)
    return null;
}
```

The first and third branches look like they should be `return field.symbol`,
not `return symbol`. As written, they only ever return null from those paths
(the middle branch is fine — `symbol` is assigned just before).

I think this is a real bug. Caveat: TAWK may have a `use field` semantics I
haven't internalized that makes `symbol` resolve through `field` in those
contexts — but no `use field` block is in scope here, and the pattern doesn't
match anywhere else in the file. **Flagging for Tony's eye.**

Note that `findSymbol` is only called rarely (grep across .twk shows no callers
in the recon scope), so if this is a bug, it hasn't been load-bearing. But if
it gets called more in future work, this would bite.

### 5.5 Two name-spaces conflated in `InstanceTable.instances`

InstanceTable.twk:25–27, 39–41.

```
/*  In order to handle function pointers, method names are added into
    the variable name space (may want to maintain a separate namespace??)
*/
if symbol.isMethod && symbol.parameters
    instances.push(symbol.gitMethodName(),(void*)instance);
instances.push(symbol.name,(void*)instance);
```

Methods get two entries: under the bare name AND under the mangled form.
This means a variable named exactly the same as some method's mangled signature
would collide. Unlikely in practice (mangled names contain parens) but
theoretically a hazard. The comment already flags this as a known smell.

### 5.6 Static type-table population in `SymbolType.setTypeTable`

SymbolType.twk:1045–1185.

The static initializers reuse the same C++ variable across two
`types.getType(...)` calls:

```
charType = types.getType("unsigned char");   // initialize unsigned-char attributes
...
charType = types.getType("char");            // re-bind; initialize char attributes
```

Both types do get registered (because `types.getType` is get-or-create), but the
static pointer `charType` ends up referring to the second registration only.
Same trick for `shortType`, `intType`, `longType`, `doubleType`. Functional but
confusing on read; a future Clay/Tony will need to know that `charType` refers
to the *signed* char and the unsigned variant is reachable only via
`types.getType("unsigned char")`.

The pattern `pointerType = longType` (line 1120) is an explicit alias — also
worth knowing: pointers are typed as unsigned long internally.

### 5.7 `Instance.checkOverload` is long, branchy, and gotos

Instance.twk:326–500.

170-line method with `startOver:` and `finish:` labels and a `goto startOver`.
Handles four resolution paths (overloaded verb, overloaded `[]`, overloaded
prefix/postfix, overloaded `()` on non-method) interleaved with chained
overload handling. Modifies `this` in place via `*this = *instance`.

This is "I'm doing more than my name suggests" territory. The method name
suggests a yes/no check; the behavior is full resolution-and-transformation.
A future refactor that splits this into per-case methods would clarify the
intent significantly, but would have to preserve the in-place mutation pattern
the caller relies on.

### 5.8 `getType()` has a `selectorType` special case

Instance.twk:804.

```
SymbolType getType()
{
    if cast       return cast.getType();
    or type       return type;
    or symbol     return symbol.type;
    or express    return express.getType();
    or statement && statement.first return statement.first.getType();
    or isSelector return selectorType;       // ← special case
    if isError    return SymbolType.nullType;
    return null;
}
```

`isSelector` is an Objective-C concept. The case is buried at the end of a
chain of "or" clauses — easy to miss when reasoning about getType's behavior
for non-OC code paths.

### 5.9 The `use field` quirk from CLAUDE.md autopsy

Tokf/CLAUDE.md:81 lists "Empty `//` comment lines reset field resolution
context — remove from method bodies" as a known issue. That's about TAWK's
*own* parsing of `.twk` source. It is not about the resolution machinery this
recon describes — but it's worth a flag: if someone is debugging "why does my
.twk file emit wrong code", that quirk is at a different layer (parser) than
this one (semantic resolution).

---

## 6. Risk Areas for Future Refactor

If a refactor of field resolution is on the table, here's what would break or
demand careful handling. Roughly ordered by surface area.

### 6.1 The `parent` chain is the public output

Every successful `find` returns an Instance with `parent` set to the route by
which it was reached. Downstream code (FormatC, Expression's checkOverload,
Instance.findGetterOrSetter) walks this chain to emit dereferences. Any change
to how `parent` is computed will ripple through code generation immediately.

In particular: `findGetterOrSetter` (Instance.twk:585) is a `while parent`
loop — it depends on the chain terminating cleanly.

### 6.2 `foundAncestor` consumers vs producers

`find` returns the instance and sets `parent = foundAncestor`. If a refactor
moves to a return-tuple/struct pattern, callers that currently rely on the
parent being attached for them will need updating. Conversely, if a refactor
internalizes parent-setting more aggressively, the `foundAncestor` field on
InstanceTable becomes vestigial.

### 6.3 Method dual-registration

Methods registered under both `name` and mangled `gitMethodName()` (and a
third name for OC: `getOCmethodName()`). Changing the lookup strategy needs
to preserve all three lookup keys, OR migrate every caller to a single key
form. Doable but not local.

### 6.4 Type-flagging during walk

`isFlagged` and `isFilled` are state mutated on SymbolType during resolution.
A refactor toward immutability would need to thread "visited set" state
through the call signatures, expanding every method's arity.

### 6.5 Cached `nullInstance` entries

`fillComponentFields` will cache misses as `nullInstance` in
`componentFields[text]`. Any refactor that wants to invalidate the cache
needs an explicit invalidation method, OR the cache needs to move to a
build-once-at-end-of-parse phase.

### 6.6 `Instance.cast` semantics

`getType()` returns `cast.getType()` when cast is set. The cast is built by
multiple methods (checkCast overloads, castAlias, checkOverload). A change to
cast handling needs all four callers in sync.

### 6.7 Coupling between Instance, Symbol, SymbolType

These classes co-mutate freely: SymbolType.add() sets fields on its Symbol
argument (`use item` in line 118 — sets `parentClass`, `isProper`, `isOCfield`,
`symbolOffset`, `symbolBitOffset`, etc.). Symbol.extendType() reaches into the
first-parameter-type's `methods` table to inject. Instance.error() rewrites
the symbol pointer to a fresh error Symbol.

Refactor toward stronger encapsulation would need to identify all the
cross-class mutations and provide explicit APIs for each.

### 6.8 The `#searchForField-` macro

If this becomes a method (per §5.2), the four loop-variable contract becomes
a return value contract. The caller's update pattern needs rework too
(currently the macro updates `lowest`, `last`, `lastParent` in place).

---

## 7. File-by-File Notes

Brief annotations for each file visited. Read alongside the file itself.

### Instance.twk (1298 lines)

Polymorphic AST node. See §4 Instance for the role. Notable methods beyond
what §4 covered:

- `castAlias()` (line 163) — if alias type differs from source type, build
  a cast Expression
- `checkBracketEqual()` (line 213) — handles `obj[i] = v` overload of `[]=`
- `checkCast(target)` (lines 244, 292) — two overloads; build a cast if types
  don't match
- `error(text)` (line 568) — rewrite symbol to a fresh `Symbol(message, nullType)`
- `insertParentAsParameter()` (line 888) — for C-class methods: walk parent
  chain to find class-type ancestor, make it the first parameter
- `setDefaults(tok)` (line 1062) — alias default-parameter insertion at call site
- `setParent(instance)` (line 1201) — assigns parent with loop detection
- `isVirtuous()` (line 926) — class-virtuosity check for overload resolution
  (virtuosity is a TAWK concept worth a separate dive if it surfaces)

### InstanceTable.twk (404 lines)

The scope stack + globals + lookup engine. Fully covered in §3 and §4.

### Symbol.twk (615 lines)

Declaration class. See §4 Symbol. Notable beyond:

- `checkParameters(parameter)` (line 123) — fix-up on method redeclaration
- `dumpDefaultName()` (line 165) — debug print showing defaults
- `externalMethodName()` (line 180) — name decoration for external C methods
  (appends parentClass.name when parentClass.isC)
- `getOCmethodName()` (line 244) — Objective-C selector-style name builder
- `getSignature(cppFlag)` (line 286) — type-only signature for use as method
  parameter (function pointer)
- `setIndirection(direct)` (line 561) — parse `*` / `&` / `^` from a PLGitem

### SymbolType.twk (1186 lines)

Class/type registry. See §4 SymbolType. Notable beyond:

- `add(item)` (line 115) — extensive previous-symbol handling, sets up
  offsets, OC bookkeeping
- `addAncestorTypes()` (line 232) — heavy descent; sets `hasDescendentTypes`
  preemptively to avoid re-entry
- `addProtocol(p)` (line 354) — Objective-C protocol registration
- `checkGetterSetter(method)` (line 375) — recognize `getFoo` / `setFoo`
  pairs and wire them as getter/setter
- `dump()`, `dumpFields()`, `dumpSymbol()` — debug output
- `getAutoGetSet()` (line 698) — walks ancestor chain for autoGetSet flag
- `handleAliasParameters(...)` (line 788) — alias with default-parameter
  remapping
- `hasParent(type)` (line 888) — ancestor-chain membership test
- `setParent(type)` (line 1013) — inheritance wiring + descendentTypes update
- `setOverloadTable()` (line 997) — initialize overloads inheriting from
  parent's overloads
- `setRefer()` (line 1026) — propagate isReferenced through inheritance + protocols
- `setTypeTable()` (line 1045) — static; primitive-type registration; the
  reused-pointer pattern is in §5.6

### Block.twk (135 lines)

Trivial container — list of statement Instances. No resolution machinery; just
shows how Instances aggregate. `getWidth()` (line 106) walks statements to
compute declaration-name indentation width — uses `getType()` on each statement
subject.

### Expression.twk (427 lines, partial read)

Bridge between Instance and the verb-subject-object model. Resolution-relevant:

- `getType()` (line 260) — defers to object/subject types
- `convert()` and related — uses `subject.getSymbol()` / `subject.findGetterOrSetter`
  to wire up implicit conversions and getter/setter dispatch (lines 114–155)
- Heavy interaction with Instance.checkOverload (§5.7)

### Tawk.twk (7325 lines, NOT deeply read — per brief scope)

Holds `currentSymbols` (InstanceTable), `currentClass` / `currentType` /
`presentClass` (SymbolType), and the call sites enumerated in §2. The actual
dispatch from parsed PLG output to InstanceTable.find lives here. Out of
scope for this recon; the entry-point table in §2 documents what's reachable
from outside.

### Other files in Tokf

Not deeply read (recon scope limited): FormatC.twk (code gen — touches
resolution only via `getSymbol`/`findGetterOrSetter` at a handful of sites),
Directive.twk (debug-injection feature; not resolution), Block.twk above.

---

## Appendix: Quick reference — "I want to find a name"

| You have… | You want… | Use |
|-----------|-----------|-----|
| a bare name in current scope | Instance (with parent chain) | `currentSymbols.find(name)` |
| a method call (Instance with mangled signature) | matching method Instance | `currentSymbols.findMethod(source)` |
| a name and a specific type to look in | Symbol on that type | `someType.findField(name)` |
| a name and a specific type, want full descent | Instance with parent chain | `someType.findFieldInstance(name)` |
| a method call against a specific type | Symbol | `someType.findMethod(source)` |
| convert from one type to another | converter method Instance | `currentSymbols.getConverter(target, source)` |
| just a Symbol, no parent chain needed | Symbol | `currentSymbols.findSymbol(name)` ⚠ see §5.4 |
| operator overload | method name String | `someType.overloaded(op)` |
| getter/setter of a parent in chain | Instance whose type has method | `someInstance.findGetterOrSetter(name)` |
