#pragma once

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
