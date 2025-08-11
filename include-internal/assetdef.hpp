#pragma once

#include <cstdint>

struct AssetHeader {
	uint32_t count;
};

struct AssetEntry {
	uint32_t id;
	uint32_t offset;
	uint32_t size;

	AssetEntry() {}
	AssetEntry(uint32_t id, uint32_t offset, uint32_t size): id(id), offset(offset), size(size) {}
};
