#include "pipeline.hpp"

#include "pipeline/buffer.hpp"
#include "pipeline/config.hpp"
#include "swapchain.hpp"

#include <ranges>
#include <string>
#include <unordered_map>

namespace graphics::pipeline {

struct Pipeline {
	const vk::Pipeline pipeline;
	const vk::PipelineLayout pipelineLayout;
	const std::vector<vk::DescriptorSetLayout> descSetLayouts;
	const std::vector<std::vector<vk::DescriptorSet>> descSets;
};

vk::DescriptorPool g_descPool;
std::unordered_map<std::string, Pipeline> g_pipelines;

void createDescriptorPool(const config::Config &config, const vk::Device &device) {
	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<std::string, uint32_t> sizesMap;
	for (const auto &n: config.pipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<std::string, uint32_t> map;
			for (const auto &b: m.bindings) {
				if (!map.contains(b.type)) {
					map.emplace(b.type, 0);
				}
				map[b.type] += b.count;
			}

			for (const auto &[k, v]: map) {
				sizesMap[k] += v * m.count;
			}
		}
	}

	// ディスクリプタセットが不要ならディスクリプタプールも不要
	if (maxSets == 0) {
		return;
	}

	// マップからベクタへ
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto &[k, v]: sizesMap) {
		poolSizes.emplace_back(
			k == "combined-image-sampler"
				? vk::DescriptorType::eCombinedImageSampler
				: k == "uniform-buffer"
				? vk::DescriptorType::eUniformBuffer
				: k == "storage-buffer"
				? vk::DescriptorType::eStorageBuffer
				: k == "input-attachment"
				? vk::DescriptorType::eInputAttachment
				: throw,
			v
		);
	}

	// 作成
	const auto ci = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(maxSets)
		.setPoolSizes(poolSizes);
	g_descPool = device.createDescriptorPool(ci);
}

void createPipelines(
	const config::Config &config,
	const std::unordered_map<std::string, uint32_t> &subpassMap,
	const vk::Device &device,
	const vk::RenderPass &renderPass
) {
	// 入力アセンブリ
	const auto piasci = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	// ビューポート
	const auto imageSize = swapchain::getImageSize();
	std::vector<vk::Viewport> viewports;
	viewports.emplace_back(0.0f, 0.0f, static_cast<float>(imageSize.width), static_cast<float>(imageSize.height), 0.0f, 1.0f);
	std::vector<vk::Rect2D> scissors;
	scissors.emplace_back(vk::Offset2D(0, 0), vk::Extent2D(imageSize.width, imageSize.height));
	const auto pvsci = vk::PipelineViewportStateCreateInfo()
		.setViewports(viewports)
		.setScissors(scissors);

	// マルチサンプル
	const auto pmsci = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	// 一時情報作成
	std::vector<PipelineCreateDynamicInfo> cdis;
	for (const auto &n: config.pipelines) {
		cdis.emplace_back(n, subpassMap, device, g_descPool);
	}

	// パイプライン作成
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	for (const auto &n: cdis) {
		cis.push_back(
			vk::GraphicsPipelineCreateInfo()
				.setStages(n.psscis)
				.setPVertexInputState(&n.pvisci)
				.setPInputAssemblyState(&piasci)
				.setPViewportState(&pvsci)
				.setPRasterizationState(&n.prsci)
				.setPMultisampleState(&pmsci)
				.setPColorBlendState(&n.pcbsci)
				.setLayout(n.pipelineLayout)
				.setRenderPass(renderPass)
				.setSubpass(n.subpass)
		);
	}
	const auto pipelines = device.createGraphicsPipelines(nullptr, cis).value;

	// 作成
	for (size_t i = 0; i < pipelines.size(); ++i) {
		g_pipelines.emplace(
			config.pipelines[i].id,
			Pipeline {
				pipelines[i],
				cdis[i].pipelineLayout,
				std::move(cdis[i].descSetLayouts),
				std::move(cdis[i].descSets),
			}
		);
	}

	// 終了
	for (auto &n: cdis) {
		n.destroy(device);
	}
}

void initialize(
	const config::Config &config,
	const std::unordered_map<std::string, uint32_t> &subpassMap,
	const vk::Device &device,
	const vk::RenderPass &renderPass
) {
	if (config.pipelines.empty()) {
		return;
	}
	createDescriptorPool(config, device);
	createPipelines(config, subpassMap, device, renderPass);
}

void bind(const vk::CommandBuffer &commandBuffer, uint32_t pipelineCount, const char *const *pipelines) {
	for (uint32_t i = 0; i < pipelineCount; ++i) {
		// TODO: pipelines[i]のエラーを取る。
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, g_pipelines.at(pipelines[i]).pipeline);
	}
}

void bindDescriptorSets(
	const vk::CommandBuffer &commandBuffer,
	const char *id,
	uint32_t count,
	uint32_t const *indices
) {
	std::vector<vk::DescriptorSet> sets;
	for (uint32_t i = 0; i < count; ++i) {
		sets.push_back(g_pipelines.at(id).descSets.at(i).at(indices[i]));
	}
	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		g_pipelines.at(id).pipelineLayout,
		0,
		sets.size(),
		sets.data(),
		0,
		nullptr
	);
}

void updateBufferDescriptor(
	const vk::Device &device,
	const char *bufferId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding
) {
	const auto &buffer = buffer::get(bufferId);
	const auto bi = vk::DescriptorBufferInfo(buffer.buffer, 0, vk::WholeSize);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(g_pipelines.at(pipelineId).descSets.at(set).at(index))
		.setDstBinding(binding)
		.setDescriptorCount(1)
		.setDescriptorType(buffer.isStorage ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer)
		.setBufferInfo(bi);
	device.updateDescriptorSets(1, &ds, 0, nullptr);
}

void terminate(const vk::Device &device) {
	for (auto &n: g_pipelines) {
		device.destroyPipeline(n.second.pipeline);
		device.destroyPipelineLayout(n.second.pipelineLayout);
		for (auto &m: n.second.descSetLayouts) {
			device.destroyDescriptorSetLayout(m);
		}
	}
	g_pipelines.clear();

	if (g_descPool) {
		device.destroyDescriptorPool(g_descPool);
		g_descPool = nullptr;
	}
}

} // namespace graphics::pipeline
