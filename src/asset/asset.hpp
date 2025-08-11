#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace asset {

void initialize(const std::string &passphrase);

std::vector<unsigned char> getAsset(uint32_t id);

} // namespace asset
