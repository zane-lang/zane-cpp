# zane-cpp design notes

## Scope and source of truth

This repository follows the Zane spec from the pinned submodule at `third_party/zane-spec` (currently commit `a1921dd1bb923f2608d95893a0cee7d9ee660802`). The surface syntax reference in `third_party/zane-spec/syntax.md` is the canonical source for language forms, and the semantic documents below drive the v0.1 feature selection.

At this spec revision, `syntax.md` §1.4, `oop.md` §2.2, and `memory_model.md` §2.10 together make Zane structs a transitively value-only storage form: a Zane struct may contain only primitives and other structs, and it becomes illegal as soon as any nested field path reaches a class or `ref`.

v0.1 intentionally targets a tiny header-only helper library. The goal is to capture a few Zane idioms that map cleanly to portable C++20 without pretending to be a compiler, runtime, or source-to-source translation layer.

## Selection principles

- Prefer explicit ownership and error-flow helpers over syntax tricks.
- Keep the public API small enough to audit in one sitting.
- Avoid exceptions and dynamic polymorphism in the library surface.
- Use macros only when the language feature cannot be modeled otherwise; v0.1 avoids macros entirely.
- Reject features that would require inventing semantics not promised by the spec.

## Inventory and decisions

| Feature / idiom | Spec reference | C++ approach | Limitations / tradeoffs | Decision |
|---|---|---|---|---|
| Rebindable non-owning `ref` storage | `syntax.md` §2.3, `memory_model.md` §2.4-§2.9 | `zane::ref<T>` is a non-null pointer wrapper with `operator*`, `operator->`, `get()`, and rebinding assignment. | C++ cannot enforce Zane's place-expression rules, lexical lifetime checks, or anchor updates across moves. The wrapper improves intent but not compile-time lifetime safety. | Implemented — small, useful, and honest about the gap. |
| Bifurcated return paths with mandatory local handling | `syntax.md` §5, `error_handling.md` §1-§6 | `zane::abortable<T, E>` stores either a success value or an abort value. `resolve(...)`, `handle(...)`, and `then(...)` provide inline recovery and success-path transformation without exceptions. | C++ cannot force every call site to attach a handler, and it cannot express Zane's `return`/`abort` inside handlers as syntax. The API captures the shape of the flow, not the enforcement level. | Implemented — this is one of the strongest spec-to-C++ translations in v0.1. |
| Payload-free `Void` style values | `syntax.md` §2.1, `error_handling.md` §2.2 and §4 | `zane::unit` and `zane::void_value` model a concrete value for payload-free success paths. | C++ `void` still exists and remains the better choice for ordinary functions. `unit` is only for value contexts such as `abortable<unit, E>`. | Implemented — tiny and clarifies the abortable API. |
| Pipe syntax | `syntax.md` §4.4 | `zane::pipe(fn)` produces a pipeline stage so ordinary C++ expressions can chain with `|`. | C++ operator lookup means the right-hand side must be a `zane::pipe(...)` stage, not an arbitrary lambda block. This is close ergonomically but not syntax-identical. | Implemented — low-cost and expressive. |
| Fixed-size `Array[size]<T>` idiom | `syntax.md` §2.6, `type_parameters.md` §7 | `zane::Array<N, T>` aliases `std::array<T, N>`, and `zane::array(...)` deduces a fixed-size array value. | C++ template order is `Array<N, T>` instead of `Array[size]<T>`, and the alias cannot enforce all Zane const-parameter naming rules. | Implemented — easy, familiar, and zero-overhead. |
| Explicit `mut` call markers (`:` vs `!`) | `syntax.md` §3.2 and §4.2, `oop.md` §4 | None in v0.1. C++ does not have custom call punctuation, and simulating it would require macros or proxy objects that obscure normal code. | Any faithful emulation would be syntactic theater rather than a clean library abstraction. | Rejected for v0.1 — too intrusive for too little value. |
| Fields-only type bodies and package-scope behavior | `oop.md` §1-§5 | Documented as a usage convention: prefer plain structs/classes for storage and free functions with receiver-first signatures for behavior. | This is stylistic rather than enforceable in a header-only library. | Planned — worth documenting, but not as a library primitive. |
| Single-assignment owners and caller-visible downgrade to refs after moves | `memory_model.md` §2.1-§3.6 | No direct helper in v0.1. | Safe C++ emulation would need ownership graph tracking, move hooks, and likely a runtime layer. A partial wrapper would imply guarantees it cannot actually keep. | Rejected for v0.1 — too much machinery for an honest library. |
| Anchors that keep refs valid across moves | `memory_model.md` §6 | None. | C++ object moves do not expose the hooks needed to retroactively retarget all references without controlling allocation and object representation. | Rejected for v0.1 — would require a custom runtime and different object model. |
| `init{}` constructor shorthand / field constructors | `syntax.md` §3.3-§3.6, `oop.md` §3 | None. | Library helpers here would mostly rename aggregate initialization instead of adding real capability. | Planned — better expressed as documentation and examples than as code. |
| Non-capturing, explicitly typed lambdas | `syntax.md` §3.7, `oop.md` §7 | None. | C++ lambdas capture by default and cannot be meaningfully constrained by a header-only helper library. | Rejected for v0.1 — enforceability is too weak. |

## Implemented API summary

### `zane::ref<T>`

Use `zane::ref<T>` when you want an explicit, rebindable, non-owning reference slot in C++ instead of a raw pointer. It is intentionally small: construct from an lvalue, rebind by assignment, and use it like a reference with `*` or `->`.

Because the current spec forbids classes and `ref`s inside Zane structs transitively, a C++ type that stores `zane::ref<T>` should be documented as a Zane class analogue or as ordinary C++ scaffolding, not as a Zane-struct analogue.

### `zane::abortable<T, E>`

Use `zane::abortable<T, E>` when a function should return either a success value or a typed abort value. The companion helpers map onto common Zane call-site patterns:

- `resolve(result, fallback)` approximates `expr ?? fallback`
- `handle(result, handler)` approximates `expr ? err { resolve ... }`
- `then(result, fn)` transforms only the success path and propagates the abort value unchanged

### `zane::pipe(...)`

Use `zane::pipe(...)` for expression pipelines when you want multi-step transformations to stay visually adjacent, similar to Zane's pipe block syntax.

### `zane::Array<N, T>` and `zane::array(...)`

These helpers give a Zane-flavored spelling for fixed-size arrays while staying a zero-overhead wrapper around `std::array`.

## Why the feature set stays small

The spec contains ideas that are much stronger than ordinary C++ library abstractions: lexical lifetime enforcement, exact abort handling guarantees, explicit mutating call markers, and anchor-based refs. v0.1 avoids pretending those are solved when they are not. The current library only includes helpers that remain transparent about what C++ can and cannot guarantee.
