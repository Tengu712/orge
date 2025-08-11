#pragma once

#include <cstdint>
#include <span>

namespace asset {

void initialize();

std::span<const unsigned char> getConfigData();

std::span<const unsigned char> getAsset(uint32_t id);

} // namespace asset
