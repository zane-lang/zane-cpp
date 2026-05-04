#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace zane {
template <std::size_t Size, class T>
using Array = std::array<T, Size>;

template <class First, class... Rest>
[[nodiscard]] constexpr auto array(First&& first, Rest&&... rest)
	-> Array<1 + sizeof...(Rest), std::common_type_t<std::remove_cvref_t<First>, std::remove_cvref_t<Rest>...>> {
	using value_type = std::common_type_t<std::remove_cvref_t<First>, std::remove_cvref_t<Rest>...>;
	return {
		value_type(std::forward<First>(first)),
		value_type(std::forward<Rest>(rest))...
	};
}
} // namespace zane
