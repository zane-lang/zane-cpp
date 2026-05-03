#pragma once

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <test_harness.hpp>

namespace zane::tests {
struct parse_error {
	std::string_view message;

	constexpr auto operator==(const parse_error&) const -> bool = default;
};

static_assert(std::is_same_v<decltype(zane::array(1, 2, 3)), zane::Array<3, int>>);

inline void test_unit(suite& suite) {
	ZANE_EXPECT(suite, zane::unit{} == zane::void_value);
}

inline void test_ref(suite& suite) {
	int first = 3;
	int second = 9;
	zane::ref current{first};

	ZANE_EXPECT_EQ(suite, *current, 3);
	*current = 4;
	ZANE_EXPECT_EQ(suite, first, 4);

	current = second;
	*current = 10;
	ZANE_EXPECT_EQ(suite, second, 10);

	zane::ref<const int> view = current;
	ZANE_EXPECT_EQ(suite, view.get(), 10);
	ZANE_EXPECT(suite, current == zane::ref{second});
}

inline void test_abortable(suite& suite) {
	auto ok = zane::abortable<int, parse_error>::success(42);
	ZANE_EXPECT(suite, ok.has_value());
	ZANE_EXPECT_EQ(suite, ok.value(), 42);

	auto failure = zane::abortable<int, parse_error>::abort({"bad"});
	ZANE_EXPECT(suite, failure.has_error());
	ZANE_EXPECT_EQ(suite, failure.error().message, std::string_view{"bad"});

	ZANE_EXPECT_EQ(suite, zane::resolve(zane::abortable<int, parse_error>::abort({"bad"}), 7), 7);
	ZANE_EXPECT_EQ(suite, zane::handle(zane::abortable<int, parse_error>::abort({"oops"}), [](parse_error error) {
		return static_cast<int>(error.message.size());
	}), 4);

	auto mapped = zane::then(zane::abortable<int, parse_error>::success(6), [](int value) {
		return value * 2;
	});
	ZANE_EXPECT(suite, mapped.has_value());
	ZANE_EXPECT_EQ(suite, mapped.value(), 12);

	auto propagated = zane::then(zane::abortable<int, parse_error>::abort({"propagate"}), [](int value) {
		return value * 2;
	});
	ZANE_EXPECT(suite, propagated.has_error());
	ZANE_EXPECT_EQ(suite, propagated.error().message, std::string_view{"propagate"});
}

inline void test_array(suite& suite) {
	constexpr auto numbers = zane::array(1, 2, 3);
	ZANE_EXPECT_EQ(suite, numbers.size(), static_cast<std::size_t>(3));
	ZANE_EXPECT_EQ(suite, numbers[0], 1);
	ZANE_EXPECT_EQ(suite, numbers[2], 3);

	auto words = zane::array(std::string_view{"za"}, std::string_view{"ne"});
	ZANE_EXPECT_EQ(suite, words[0], std::string_view{"za"});
	ZANE_EXPECT_EQ(suite, words[1], std::string_view{"ne"});
}

inline void test_pipe(suite& suite) {
	auto doubled = 21 | zane::pipe([](int value) {
		return value * 2;
	});
	ZANE_EXPECT_EQ(suite, doubled, 42);

	auto length = std::string{"zane"}
		| zane::pipe([](std::string value) {
			return value + "-cpp";
		})
		| zane::pipe([](const std::string& value) {
			return value.size();
		});
	ZANE_EXPECT_EQ(suite, length, static_cast<std::size_t>(8));

	auto recovered = zane::abortable<int, parse_error>::success(21)
		| zane::pipe([](auto result) {
			return zane::then(std::move(result), [](int value) {
				return value * 2;
			});
		})
		| zane::pipe([](auto result) {
			return zane::resolve(std::move(result), 0);
		});
	ZANE_EXPECT_EQ(suite, recovered, 42);
}

inline auto run_all_tests() -> int {
	suite suite{};
	test_unit(suite);
	test_ref(suite);
	test_abortable(suite);
	test_array(suite);
	test_pipe(suite);
	return suite.failures() == 0 ? 0 : 1;
}
} // namespace zane::tests
