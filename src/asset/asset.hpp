#pragma once

#include <cstdint>
#include <vector>

namespace asset {

void initialize();

std::vector<unsigned char> getConfigData();

std::vector<unsigned char> getAsset(uint32_t id);

} // namespace asset
