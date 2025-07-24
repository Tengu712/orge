#pragma once

#include <string>

namespace error {

enum class SpecialError {
	NeedRecreateSwapchain,
	NeedRecreateSurface,
};

void setMessage(const std::string &e);

const std::string &getMessage();

} // namespace error
