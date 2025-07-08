#include "pipeline.hpp"

#include "swapchain.hpp"

#include <fstream>
#include <ranges>
#include <string>
#include <unordered_map>

namespace graphics::pipeline {

vk::DescriptorPool g_descPool;
std::vector<vk::Pipeline> g_pipelines;
// パイプラインIDとパイプライン配列へのインデックスを対応付けるマップ
std::unordered_map<std::string, size_t> g_pipelineMap;

vk::ShaderModule createShaderModule(const vk::Device &device, const std::string &path) {
	std::fstream file(path, std::ios::in | std::ios::binary);
	if (!file) {
		return nullptr;
	}

	file.seekg(0, std::ios::end);
	const auto size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size <= 0 || size % sizeof(uint32_t) != 0) {
		return nullptr;
	}

	std::vector<uint32_t> code(size / sizeof(uint32_t));
	file.read(reinterpret_cast<char *>(code.data()), size);

	if (!file || file.gcount() != size) {
		return nullptr;
	}

	const auto ci = vk::ShaderModuleCreateInfo()
		.setCode(code);
	return device.createShaderModule(ci);
}

// NOTE: パイプラインを一気に作成する都合で、forスコープを抜けて参照がぶっ壊れるので。
struct PipelineCreateTempInfo {
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
	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> pcbass;
	vk::PipelineColorBlendStateCreateInfo pcbsci;
	// ディスクリプタセットレイアウト
	std::vector<vk::DescriptorSetLayout> descSetLayouts;
	// パイプラインレイアウト
	vk::PipelineLayout pipelineLayout;
	// サブパスID
	uint32_t subpass;

	PipelineCreateTempInfo() = delete;

	PipelineCreateTempInfo(const PipelineConfig &config, const vk::Device &device) {
		// シェーダステージ
		vertexShader = createShaderModule(device, config.vertexShader);
		fragmentShader = createShaderModule(device, config.fragmentShader);
		if (!vertexShader || !fragmentShader) {
			throw "failed to create shader modules.";
		}
		psscis.emplace_back(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			vertexShader,
			"main"
		);
		psscis.emplace_back(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			fragmentShader,
			"main"
		);

		// 頂点入力
		uint32_t sum = 0;
		for (uint32_t i = 0; i < config.vertexInputAttributes.size(); ++i) {
			const auto &n = config.vertexInputAttributes.at(i);
			viads.emplace_back(
				i,
				0,
				n == 1 ? vk::Format::eR32Sfloat
				: n == 2 ? vk::Format::eR32G32Sfloat
				: n == 3 ? vk::Format::eR32G32B32Sfloat
				: vk::Format::eR32G32B32A32Sfloat,
				sizeof(float) * sum
			);
			sum += n;
		}
		vibds.emplace_back(0, sizeof(float) * sum, vk::VertexInputRate::eVertex);
		pvisci = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(vibds)
			.setVertexAttributeDescriptions(viads);

		// ラスタライゼーション
		prsci = vk::PipelineRasterizationStateCreateInfo()
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(config.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f);

		// カラーブレンド
		for (const auto &n: config.colorBlends) {
			pcbass.emplace_back(
				n ? vk::True : vk::False,
				vk::BlendFactor::eSrcAlpha,
				vk::BlendFactor::eOneMinusSrcAlpha,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eSrcAlpha,
				vk::BlendFactor::eOneMinusSrcAlpha,
				vk::BlendOp::eAdd,
				vk::ColorComponentFlagBits::eR
				| vk::ColorComponentFlagBits::eG
				| vk::ColorComponentFlagBits::eB
				| vk::ColorComponentFlagBits::eA
			);
		}
		pcbsci = vk::PipelineColorBlendStateCreateInfo()
			.setAttachments(pcbass);

		// ディスクリプタセットレイアウト
		for (const auto &n: config.descSets) {
			const auto ci = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(n.bindings);
			descSetLayouts.push_back(device.createDescriptorSetLayout(ci));
		}

		// パイプラインレイアウト
		const auto plci = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(descSetLayouts);
		pipelineLayout = device.createPipelineLayout(plci);

		// サブパスID
		subpass = config.subpass;
	}

	void destroy(const vk::Device &device) {
		device.destroyPipelineLayout(pipelineLayout);
		for (auto &n: descSetLayouts) {
			device.destroyDescriptorSetLayout(n);
		}
		device.destroyShaderModule(fragmentShader);
		device.destroyShaderModule(vertexShader);
	}
};

void createPipelines(const Config &config, const vk::Device &device, const vk::RenderPass &renderPass) {
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
	std::vector<PipelineCreateTempInfo> cits;
	for (const auto &n: config.pipelines) {
		g_pipelineMap.emplace(n.id, g_pipelineMap.size());
		cits.emplace_back(n, device);
	}

	// 作成情報作成
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	for (const auto &n: cits) {
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
	g_pipelines = device.createGraphicsPipelines(nullptr, cis).value;

	// チェック
	if (g_pipelines.size() != g_pipelineMap.size()) {
		throw "something is wrong with creating graphics pipelines.";
	}

	// 終了
	for (auto &n: cits) {
		n.destroy(device);
	}
}

void createDescriptorPool(const Config &config, const vk::Device &device) {
	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<vk::DescriptorType, uint32_t> bindingMap;
	for (const auto &m: config.pipelines) {
		for (const auto &n: m.descSets) {
			maxSets += n.count;

			std::unordered_map<vk::DescriptorType, uint32_t> map;
			for (const auto &b: n.bindings) {
				if (!map.contains(b.descriptorType)) {
					map.emplace(b.descriptorType, 0);
				}
				map[b.descriptorType] += b.descriptorCount;
			}

			for (const auto &[k, v]: map) {
				bindingMap[k] += v * n.count;
			}
		}
	}

	// ディスクリプタセットが不要ならディスクリプタプールも不要
	if (maxSets == 0) {
		return;
	}

	// マップからベクタへ
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto &[k, v]: bindingMap) {
		poolSizes.emplace_back(k, v);
	}

	// 作成
	const auto ci = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(maxSets)
		.setPoolSizes(poolSizes);
	g_descPool = device.createDescriptorPool(ci);
}

void initialize(const Config &config, const vk::Device &device, const vk::RenderPass &renderPass) {
	if (config.pipelines.empty()) {
		return;
	}
	createDescriptorPool(config, device);
	createPipelines(config, device, renderPass);
}

void bind(const vk::CommandBuffer &commandBuffer, uint32_t pipelineCount, const char *const *pipelines) {
	for (uint32_t i = 0; i < pipelineCount; ++i) {
		// TODO: pipelines[i]のエラーを取る。
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, g_pipelines.at(g_pipelineMap.at(pipelines[i])));
	}
}

void terminate(const vk::Device &device) {
	for (auto &n: g_pipelines) {
		device.destroyPipeline(n);
	}
	g_pipelines.clear();

	g_pipelineMap.clear();
}

} // namespace graphics::pipeline
