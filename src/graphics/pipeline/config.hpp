#pragma once

#include "../../config.hpp"

#include <vulkan/vulkan.hpp>

namespace graphics::pipeline {

// パイプライン作成における流動的な情報
//
// 要は、パースされたconfigをもとに、使える情報にさらに変換されたもの。
// ……という体の、コードの煩雑さを抑えるためのオブジェクト。
// こうして切り出さないと、特に一度にすべてパイプラインを作成する都合で、とても煩雑になるので。
struct PipelineCreateDynamicInfo {
	/* 一時オブジェクト */

	// シェーダステージ
	vk::ShaderModule vertexShader;
	vk::ShaderModule fragmentShader;
	std::vector<vk::PipelineShaderStageCreateInfo> psscis;

	// 頂点入力
	std::vector<vk::VertexInputAttributeDescription> viads;
	std::vector<vk::VertexInputBindingDescription> vibds;
	vk::PipelineVertexInputStateCreateInfo pvisci;

	// ラスタライゼーション
	vk::PipelineRasterizationStateCreateInfo prsci;

	// デプスステンシル
	vk::PipelineDepthStencilStateCreateInfo pdssci;

	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> pcbass;
	vk::PipelineColorBlendStateCreateInfo pcbsci;

	// サブパスID
	uint32_t subpass;

	/* オブジェクト */

	vk::PipelineLayout pipelineLayout;
	std::vector<vk::DescriptorSetLayout> descSetLayouts;
	std::vector<std::vector<vk::DescriptorSet>> descSets;

	/* コンストラクタ・デストラクタ */

	PipelineCreateDynamicInfo() = delete;
	PipelineCreateDynamicInfo(
		const config::PipelineConfig &config,
		const std::unordered_map<std::string, uint32_t> &subpassMap,
		const vk::Device &device,
		const vk::DescriptorPool &descPool
	);

	// 一時オブジェクトを破棄する
	void destroy(const vk::Device &device);
};

} // namespace graphics::pipeline
