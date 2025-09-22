#pragma once

#include <cstdint>
#include <span>
#include <string>

namespace asset {

void initialize(const std::string &path);

std::span<const unsigned char> getAsset(uint32_t id);

} // namespace asset
