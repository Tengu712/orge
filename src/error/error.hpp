#pragma once

#include <format>
#include <string>

namespace error {

template<typename T>
std::string enumToInt64String(const T &n) {
	if constexpr (std::is_enum_v<T>) {
		return std::to_string(static_cast<int64_t>(n));
	} else {
		return std::format("{}", n);
	}
}

template<typename T, typename U>
const auto &at(const T &container, const U &key, const std::string &subject) {
	try {
		return container.at(key);
	} catch (const std::out_of_range &) {
		throw std::out_of_range(std::format("the key '{}' is invalid for {}.", enumToInt64String(key), subject));
	}
}

enum class SpecialError {
	NeedRecreateSwapchain,
	NeedRecreateSurface,
};

void setMessage(const std::string &e);

const std::string &getMessage();

} // namespace error
