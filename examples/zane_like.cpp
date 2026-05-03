#include <iostream>
#include <string_view>

#include <zane-cpp.hpp>

namespace demo {
struct Engine {
	int rpm = 0;
};

struct Car {
	zane::ref<Engine> engine;
};

struct parse_error {
	std::string_view message;
};

auto parse_scale(std::string_view text) -> zane::abortable<int, parse_error> {
	if (text == "high") {
		return zane::abortable<int, parse_error>::success(3);
	}
	return zane::abortable<int, parse_error>::abort({"expected "high""});
}

auto scaled_rpm(Car car, std::string_view text) -> int {
	return parse_scale(text)
		| zane::pipe([&](auto result) {
			return zane::then(std::move(result), [&](int scale) {
				return car.engine->rpm * scale;
			});
		})
		| zane::pipe([](auto result) {
			return zane::handle(std::move(result), [](const parse_error&) {
				return 0;
			});
		});
}
} // namespace demo

int main() {
	demo::Engine engine{900};
	demo::Car car{engine};
	auto lanes = zane::array(1, 2, 3);

	std::cout << "scaled rpm: " << demo::scaled_rpm(car, "high") << '\n';
	std::cout << "fallback rpm: " << demo::scaled_rpm(car, "low") << '\n';
	std::cout << "array lanes: " << lanes.size() << '\n';
}
