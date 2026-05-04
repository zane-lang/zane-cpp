# zane-cpp

`zane-cpp` is a small header-only C++20 helper library for writing C++ in a style that feels closer to the [Zane language spec](https://github.com/zane-lang/spec). Version `0.1.0` focuses on a tiny, polished surface instead of trying to emulate the whole language.

## Spec target

`zane-cpp` is currently aligned with [4a11ec9](https://github.com/zane-lang/spec/commit/4a11ec9a7b26fdf5993977603b24ce2e7582a6d2).

The pinned spec checkout lives in `third_party/zane-spec/`, and `docs/design.md` documents mappings and tradeoffs relative to that revision.

## What v0.1 implements

- `zane::ref<T>` for explicit non-owning, rebindable references
- `zane::abortable<T, E>` plus `resolve`, `handle`, and `then` for Zane-inspired bifurcated return flows without exceptions
- `zane::pipe(...)` for pipeline-style expression chaining
- `zane::Array<N, T>` and `zane::array(...)` for fixed-size array values
- `zane::unit` / `zane::void_value` for payload-free success values

See [`docs/design.md`](docs/design.md) for the full spec-to-C++ mapping, tradeoffs, and planned or rejected ideas.

## Single-header usage

The distributable header is:

```cpp
#include <zane-cpp.hpp>
```

The modular source root used to generate it is:

```text
include-src/zane/prelude.hpp
```

### Minimal example

```cpp
#include <zane-cpp.hpp>
#include <string_view>

struct ParseError {
    std::string_view message;
};

auto parse_scale(std::string_view text) -> zane::abortable<int, ParseError> {
    if (text == "high") {
        return zane::abortable<int, ParseError>::success(3);
    }
    return zane::abortable<int, ParseError>::abort({"expected \"high\""});
}

int main() {
    auto value = parse_scale("low")
        | zane::pipe([](auto result) {
            return zane::handle(std::move(result), [](const ParseError&) {
                return 0;
            });
        });

    auto refs = zane::array(1, 2, 3);
    return value + refs[0];
}
```

A runnable example lives at [`examples/zane_like.cpp`](examples/zane_like.cpp).

## Development workflow

### Build the single header

```bash
devbox run -- just build-header
```

Exact Quom invocation:

```bash
devbox run -- python -m quom --include_directory include-src include-src/zane/prelude.hpp include/zane-cpp.hpp
```

### Run tests

```bash
devbox run -- just test
```

The test suite compiles and runs two binaries:

- one against the modular root header from `include-src/`
- one against the generated `include/zane-cpp.hpp` single header

### Run the full check suite

```bash
devbox run -- just check
```

`just check` rebuilds the single header, runs the tests, and compiles plus runs the example program.

## Repository layout

```text
include-src/zane/     modular headers
include/zane-cpp.hpp  generated distributable header
tests/                offline-friendly test harness and coverage
examples/             example programs
docs/design.md        spec-driven design notes and decisions
third_party/zane-spec pinned spec submodule
```
