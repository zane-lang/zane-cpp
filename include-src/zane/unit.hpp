#pragma once

#include <compare>

namespace zane {
struct unit {
	constexpr auto operator<=>(const unit&) const = default;
};

inline constexpr unit void_value{};
} // namespace zane
