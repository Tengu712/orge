#pragma once

#include "image.hpp"

namespace graphics::resource {

void destroyAllAttachmentImages() noexcept;

void initializeAllAttachmentImages();

const Image &getAttachmentImage(uint32_t index, const std::string &id);

} // namespace graphics::resource
