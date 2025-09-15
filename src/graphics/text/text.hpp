#pragma once

#include <orge.h>
#include <string>
#include <vector>

namespace graphics::text {

struct TextRenderingInstance {
	alignas(16) float transform[16];
	alignas(16) float uv[4];
	alignas(16) uint32_t texId[4];
};

void destroyTextRenderingResources();

void initializeTextRenderingResources();

void rasterizeText(const std::string &fontId, const std::string &text);

void clearLayoutContext();

void layoutText(
	const std::string &renderPassId,
	const std::string &subpassId,
	const std::string &fontId,
	const std::string &text,
	float x,
	float y,
	float height,
	OrgeTextLocationHorizontal horizontal,
	OrgeTextLocationVertical vertical
);

const std::vector<std::pair<size_t, size_t>> &getIndices(const std::string &renderPassId, uint32_t subpassIndex);

} // namespace graphics::text
