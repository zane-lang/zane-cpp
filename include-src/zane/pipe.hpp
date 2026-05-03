#pragma once

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace zane {
template <class Function>
class pipe_stage {
public:
	constexpr explicit pipe_stage(Function function)
		: function_(std::move(function)) {}

	template <class Value>
	friend constexpr decltype(auto) operator|(Value&& value, const pipe_stage& stage)
		requires std::invocable<const Function&, Value&&> {
		return std::invoke(stage.function_, std::forward<Value>(value));
	}

	template <class Value>
	friend constexpr decltype(auto) operator|(Value&& value, pipe_stage&& stage)
		requires std::invocable<Function, Value&&> {
		return std::invoke(std::move(stage.function_), std::forward<Value>(value));
	}

private:
	Function function_;
};

template <class Function>
[[nodiscard]] constexpr auto pipe(Function&& function) -> pipe_stage<std::decay_t<Function>> {
	return pipe_stage<std::decay_t<Function>>(std::forward<Function>(function));
}
} // namespace zane
