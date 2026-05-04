set shell := ["bash", "-euo", "pipefail", "-c"]

root_header := "include-src/zane/prelude.hpp"
single_header := "include/zane-cpp.hpp"
cxx := env_var_or_default("CXX", "clang++")
common_flags := "-std=c++20 -Wall -Wextra -Wpedantic -Werror -O2"

# build-header runs Quom deterministically from include-src/zane/prelude.hpp into include/zane-cpp.hpp.
build-header:
	mkdir -p include
	devbox run -- python -m quom --include_directory include-src {{root_header}} {{single_header}}

test: build-header
	mkdir -p .build/tests
	devbox run -- {{cxx}} {{common_flags}} -Iinclude-src -Itests tests/test_modular.cpp -o .build/tests/test_modular
	devbox run -- ./.build/tests/test_modular
	devbox run -- {{cxx}} {{common_flags}} -Iinclude -Itests tests/test_single_header.cpp -o .build/tests/test_single_header
	devbox run -- ./.build/tests/test_single_header

check: build-header test
	mkdir -p .build/examples
	devbox run -- {{cxx}} {{common_flags}} -Iinclude examples/zane_like.cpp -o .build/examples/zane_like
	devbox run -- ./.build/examples/zane_like
