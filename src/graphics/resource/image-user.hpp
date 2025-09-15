#pragma once

#include "image.hpp"

namespace graphics::resource {

void destroyAllUserImages() noexcept;

void addUserImageFromFile(const std::string &file);

void destroyUserImage(const std::string &id) noexcept;

const Image &getUserImage(const std::string &id);

} // namespace graphics::resource
