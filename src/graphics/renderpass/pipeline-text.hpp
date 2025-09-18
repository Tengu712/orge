#pragma once

#include "pipeline.hpp"

namespace graphics::renderpass {

// NOTE: 本当はGraphicsPipelinesを返したいが、
//       unordered_mapでemplaceできなくてデストラクタが呼ばれてしまうので、
//       仕方なく最後の引数に格納先を渡す。
//       ムーブコンストラクタ定義したりunique_ptrで管理すれば解決するんだけど、
//       なんか嫌だ。
void createTextRenderingPipeline(
	const vk::RenderPass &renderPass,
	uint32_t subpassIndex,
	std::unordered_map<uint32_t, GraphicsPipeline> &pipelines
);

} // namespace graphics::renderpass
