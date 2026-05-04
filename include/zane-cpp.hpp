#pragma once

#include <concepts>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

namespace zane {
namespace detail {
[[noreturn]] inline void panic(const char*) noexcept {
	std::abort();
}
} // namespace detail

template <class T, class E>
class abortable {
public:
	using value_type = T;
	using error_type = E;

	constexpr abortable(const abortable&) = default;
	constexpr abortable(abortable&&) noexcept = default;
	constexpr auto operator=(const abortable&) -> abortable& = default;
	constexpr auto operator=(abortable&&) noexcept -> abortable& = default;
	~abortable() = default;

	[[nodiscard]] static constexpr auto success(T value) -> abortable {
		return abortable(std::in_place_index<0>, std::move(value));
	}

	[[nodiscard]] static constexpr auto abort(E error) -> abortable {
		return abortable(std::in_place_index<1>, std::move(error));
	}

	[[nodiscard]] constexpr auto has_value() const noexcept -> bool {
		return storage_.index() == 0;
	}

	[[nodiscard]] constexpr auto has_error() const noexcept -> bool {
		return storage_.index() == 1;
	}

	[[nodiscard]] constexpr explicit operator bool() const noexcept {
		return has_value();
	}

	[[nodiscard]] constexpr auto value() & noexcept -> T& {
		auto* value = std::get_if<0>(&storage_);
		if (value == nullptr) {
			detail::panic("zane::abortable does not contain a value");
		}
		return *value;
	}

	[[nodiscard]] constexpr auto value() const& noexcept -> const T& {
		auto* value = std::get_if<0>(&storage_);
		if (value == nullptr) {
			detail::panic("zane::abortable does not contain a value");
		}
		return *value;
	}

	[[nodiscard]] constexpr auto value() && noexcept -> T&& {
		auto* value = std::get_if<0>(&storage_);
		if (value == nullptr) {
			detail::panic("zane::abortable does not contain a value");
		}
		return std::move(*value);
	}

	[[nodiscard]] constexpr auto error() & noexcept -> E& {
		auto* error = std::get_if<1>(&storage_);
		if (error == nullptr) {
			detail::panic("zane::abortable does not contain an error");
		}
		return *error;
	}

	[[nodiscard]] constexpr auto error() const& noexcept -> const E& {
		auto* error = std::get_if<1>(&storage_);
		if (error == nullptr) {
			detail::panic("zane::abortable does not contain an error");
		}
		return *error;
	}

	[[nodiscard]] constexpr auto error() && noexcept -> E&& {
		auto* error = std::get_if<1>(&storage_);
		if (error == nullptr) {
			detail::panic("zane::abortable does not contain an error");
		}
		return std::move(*error);
	}

private:
	template <std::size_t Index, class U>
	constexpr abortable(std::in_place_index_t<Index>, U&& value)
		: storage_(std::in_place_index<Index>, std::forward<U>(value)) {}

	std::variant<T, E> storage_;
};

template <class T, class E>
[[nodiscard]] constexpr auto resolve(abortable<T, E> result, T fallback) -> T {
	if (result.has_value()) {
		return std::move(result).value();
	}
	return fallback;
}

template <class T, class E, class Handler>
requires std::invocable<Handler, E&&> && std::convertible_to<std::invoke_result_t<Handler, E&&>, T>
[[nodiscard]] constexpr auto handle(abortable<T, E> result, Handler&& handler) -> T {
	if (result.has_value()) {
		return std::move(result).value();
	}
	return static_cast<T>(std::invoke(std::forward<Handler>(handler), std::move(result).error()));
}

template <class T, class E, class Function>
requires std::invocable<Function, T&&>
[[nodiscard]] constexpr auto then(abortable<T, E> result, Function&& function)
	-> abortable<std::remove_cvref_t<std::invoke_result_t<Function, T&&>>, E> {
	using next_type = std::remove_cvref_t<std::invoke_result_t<Function, T&&>>;
	if (result.has_value()) {
		return abortable<next_type, E>::success(std::invoke(std::forward<Function>(function), std::move(result).value()));
	}
	return abortable<next_type, E>::abort(std::move(result).error());
}
} // namespace zane

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

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>

namespace zane {
template <class T>
class ref {
public:
	using value_type = T;

	constexpr ref() = delete;
	constexpr ref(T& value) noexcept
		: ptr_(std::addressof(value)) {}
	constexpr ref(std::reference_wrapper<T> value) noexcept
		: ptr_(std::addressof(value.get())) {}

	template <class U>
	requires std::convertible_to<U*, T*>
	constexpr ref(ref<U> other) noexcept
		: ptr_(std::addressof(other.get())) {}

	constexpr auto operator=(T& value) noexcept -> ref& {
		ptr_ = std::addressof(value);
		return *this;
	}

	constexpr auto operator=(std::reference_wrapper<T> value) noexcept -> ref& {
		ptr_ = std::addressof(value.get());
		return *this;
	}

	[[nodiscard]] constexpr auto get() const noexcept -> T& {
		return *ptr_;
	}

	[[nodiscard]] constexpr auto operator*() const noexcept -> T& {
		return *ptr_;
	}

	[[nodiscard]] constexpr auto operator->() const noexcept -> T* {
		return ptr_;
	}

	[[nodiscard]] constexpr auto data() const noexcept -> T* {
		return ptr_;
	}

	[[nodiscard]] friend constexpr auto operator==(ref left, ref right) noexcept -> bool {
		return left.ptr_ == right.ptr_;
	}

private:
	T* ptr_;
};

template <class T>
ref(T&) -> ref<T>;

template <class T>
[[nodiscard]] constexpr auto make_ref(T& value) noexcept -> ref<T> {
	return ref<T>(value);
}
} // namespace zane

#include <compare>

namespace zane {
struct unit {
	constexpr auto operator<=>(const unit&) const = default;
};

inline constexpr unit void_value{};
} // namespace zane

#include <string_view>

namespace zane {
inline constexpr int version_major = 0;
inline constexpr int version_minor = 1;
inline constexpr int version_patch = 0;
inline constexpr std::string_view version_string = "0.1.0";
} // namespace zane

