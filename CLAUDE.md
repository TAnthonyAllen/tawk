# CLAUDE.md — TAWK Repository

This file orients Claude Code (Clod) when working in this repository.
Read `projectBible.md` for full ecosystem context (PLG/TAWK/Incant).

---

## What TAWK Is

TAWK (Tony's AWK) is a source-to-source transpiler — it reads `.twk` source files and generates C++.
It is the middle layer of the ecosystem: PLG recognizes, **TAWK transforms**, Incant reasons.

TAWK makes C++ writable in a friendlier notation. The generated C++ is what actually runs.
Nobody reads the generated C++. Everybody reads the `.twk` source.

---

## Repositories

| Repo | URL |
|------|-----|
| TAWK | https://github.com/TAnthonyAllen/tawk |
| PLG | https://github.com/TAnthonyAllen/plg |
| Incant | https://github.com/TAnthonyAllen/incant |
| Support | https://github.com/TAnthonyAllen/support |

---

## Local Directory

```
/Users/anthony/Library/CloudStorage/Dropbox/data/InProcess/Tokf/
```

---

## File Types

- `.twk` — TAWK source files. **Source of truth.** Edit these.
- `.C` / `.h` — Generated C++ output. Never edit directly.
- `.g` — PLG grammar files. TAWK uses PLG to parse `.twk` files.
- `Tawk.ext` / externals — TAWK's own external declarations (what TAWK knows about itself)

---

## The Bootstrapping Problem

TAWK is written in TAWK. To build TAWK from source you need a working TAWK binary.

**Current solution**: prebuilt binary at `~/bin/tok` (note: binary is named `tok`, source convention is `tawk`).

**The binary is named `tok`** — invoke as `tok FileName.twk` to generate `FileName.C` and `FileName.h`.

Same bootstrapping problem exists in PLG (see PLG's CLAUDE.md). Both are solved by prebuilt binaries for now.
Long term: self-hosting build story needs to be designed.

---

## TAWK Language Features

TAWK syntax compiles to C++. Key features:

- **Class definition**: `class MyClass { ... }` → C++ class
- **Inheritance**: `class Foo extends Bar` → `class Foo : public Bar`
- **Field access**: dot notation → pointer dereference (`foo.bar` → `foo->bar`)
- **`new`**: `field = new;` → `field = new ClassName();` (type inferred from declaration)
- **`use field`**: sets resolution context — `use buffer` means bare names resolve through `buffer`
- **`#autoGetSet`**: generates getter/setter methods automatically
- **Boolean lists**: `boolean { flag1 flag2 flag3 ; }` → individual `unsigned int` bitfields
- **`print(buffer) "text" value:;`** — the `:` shortcut adds a newline
- **`cerr "message":;`** — error output with newline
- **`extern` blocks**: declare free functions and class externals for TAWK's benefit
- **Method ordering**: alphabetical by convention (Anthony's preference for Xcode navigability)

---

## TAWK Known Issues (autopsy table)

See projectBible.md for the full list. Key ones:

1. **Empty `//` comment lines** reset field resolution context — remove from method bodies
2. **`new` type inference** inconsistent — use explicit `field = new ClassName()` when `field = new` fails
3. **No include search paths** — all includes must be absolute paths. No `-I` flag support.
4. **No include guards** generated in `.h` files — add manually for inherited classes
5. **`extern "C"`** in tawk-generated files gets clobbered on re-tawk — put C-linkage functions in hand-written files
6. **TAWK error propagation** — unresolved references are embedded as errors in generated C++. Xcode flags them.

---

## External Declarations Pattern

TAWK needs to know about classes it references. Externals are declared in files included at the top of `.twk` files.

```
external MyClass
{
    // class fields and methods TAWK needs to know about
    String name;
    void doSomething();
}

external MyClass.h
{
    // free functions that live in MyClass.C but outside the class
    extern int myFreeFunction(MyClass a, OtherClass b);
}
```

The `.h` external puts declarations in the generated header as free functions, not class members.
This is the pattern for `extern "C"` bridge functions (see PLG's `foundIn`).

---

## Build Workflow

```bash
# 1. Edit .twk source
# 2. Regenerate C++
tok FileName.twk        # produces FileName.C and FileName.h

# 3. Build in Xcode
# project.yml manages the Xcode project — regenerate with:
xcodegen generate

# 4. Verify test passes
```

---

## Working Relationship

**Anthony** — architect, final authority.
**Clay** (Claude at claude.ai) — design, reasoning, architecture.
**Clod** (Claude Code) — execution, file edits, GitHub, build verification.

Standing permissions: change any code in source directories without asking. Ask before GitHub pushes.

See `projectBible.md` for full glossary, HWF protocol, and ecosystem context.
