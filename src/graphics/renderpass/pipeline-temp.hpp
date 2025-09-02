#pragma once

#include <vulkan/vulkan.hpp>

namespace graphics::renderpass {

struct PipelineCreateTemporaryInfos {
	// ID
	std::string id;

	// シェーダステージ
	vk::ShaderModule vertexShader;
	vk::ShaderModule fragmentShader;
	uint32_t texCount;
	vk::SpecializationMapEntry fspme;
	vk::SpecializationInfo fspi;
	std::vector<vk::PipelineShaderStageCreateInfo> sscis;

	// 頂点入力
	std::vector<vk::VertexInputAttributeDescription> viads;
	std::vector<vk::VertexInputBindingDescription> vibds;
	vk::PipelineVertexInputStateCreateInfo visci;

	// 頂点アセンブリ
	vk::PipelineInputAssemblyStateCreateInfo iasci;

	// ラスタライゼーション
	vk::PipelineRasterizationStateCreateInfo rsci;

	// デプスステンシル
	vk::PipelineDepthStencilStateCreateInfo dssci;

	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> cbass;
	vk::PipelineColorBlendStateCreateInfo cbsci;

	// パイプラインレイアウト
	vk::PipelineLayout pipelineLayout;

	// ディスクリプタセットレイアウト
	std::vector<vk::DescriptorSetLayout> descSetLayouts;

	// サブパスID
	uint32_t subpass;

	// ディスクリプタセット
	std::vector<std::vector<vk::DescriptorSet>> descSets;

	// インプットアタッチメントがテクスチャか否か
	std::unordered_map<std::string, bool> inputAttachmentTypes;

	PipelineCreateTemporaryInfos(const std::string &pipelineId, uint32_t subpassIndex);
	~PipelineCreateTemporaryInfos();
};

} // namespace graphics::renderpass
