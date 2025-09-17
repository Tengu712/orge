#pragma once

#include "image.hpp"

namespace graphics::resource {

void destroyAllStorageImages() noexcept;

void addStorageImage(const std::string &id, uint32_t width, uint32_t height, uint32_t format);

void destroyStorageImage(const std::string &id) noexcept;

const Image &getStorageImage(const std::string &id);

} // namespace graphics::resource