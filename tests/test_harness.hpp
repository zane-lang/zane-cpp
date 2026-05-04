#pragma once

#include <iostream>
#include <string_view>

namespace zane::tests {
class suite {
public:
	constexpr suite() = default;

	constexpr void expect(bool condition, std::string_view expression, std::string_view file, int line) {
		if (!condition) {
			std::cerr << file << ':' << line << ": expected " << expression << '\n';
			++failures_;
		}
	}

	template <class Left, class Right>
	constexpr void expect_equal(const Left& left, const Right& right, std::string_view expression, std::string_view file, int line) {
		if (!(left == right)) {
			std::cerr << file << ':' << line << ": expected " << expression << '\n';
			++failures_;
		}
	}

	[[nodiscard]] constexpr auto failures() const noexcept -> int {
		return failures_;
	}

private:
	int failures_ = 0;
};
} // namespace zane::tests

#define ZANE_EXPECT(suite, expression) (suite).expect(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
#define ZANE_EXPECT_EQ(suite, left, right) (suite).expect_equal((left), (right), #left " == " #right, __FILE__, __LINE__)
