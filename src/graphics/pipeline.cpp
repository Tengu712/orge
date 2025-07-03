#include "pipeline.hpp"

#include "swapchain.hpp"

#include <fstream>
#include <ranges>
#include <string>
#include <unordered_map>

namespace graphics::pipeline {

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
	try {
		return device.createShaderModule(ci);
	} catch (...) {
		return nullptr;
	}
}

Error initialize(const Config &config_, const vk::Device &device, const vk::RenderPass &renderPass) {
	if (config_.pipelines.empty()) {
		return Error::None;
	}

	const auto imageSize = swapchain::getImageSize();
	const auto width = imageSize.width;
	const auto height = imageSize.height;

	std::vector<vk::ShaderModule> tempShaderModules;
	std::vector<vk::DescriptorSetLayout> tempDescSetLayouts;
	std::vector<vk::PipelineLayout> tempPipelineLayout;
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	cis.reserve(config_.pipelines.size());

	try {
		// すべてのパイプラインについて
		for (const auto &config: config_.pipelines) {
			// パイプラインシェーダステージ配列
			const auto vertexShader = createShaderModule(device, config.vertexShader);
			const auto fragmentShader = createShaderModule(device, config.fragmentShader);
			if (!vertexShader || !fragmentShader) {
				return Error::CreateShaderModule;
			}
			tempShaderModules.push_back(vertexShader);
			tempShaderModules.push_back(fragmentShader);
			std::vector<vk::PipelineShaderStageCreateInfo> psscis;
			psscis.push_back(
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eVertex)
					.setModule(vertexShader)
					.setPName("main")
			);
			psscis.push_back(
				vk::PipelineShaderStageCreateInfo()
					.setStage(vk::ShaderStageFlagBits::eFragment)
					.setModule(fragmentShader)
					.setPName("main")
			);

			// 頂点入力
			uint32_t sum = 0;
			std::vector<vk::VertexInputAttributeDescription> visds;
			for (uint32_t i = 0; i < config.vertexInputAttributes.size(); ++i) {
				const auto &n = config.vertexInputAttributes.at(i);
				visds.emplace_back(
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
			const auto vibd = vk::VertexInputBindingDescription(0, sizeof(float) * sum, vk::VertexInputRate::eVertex);
			const auto pvisci = vk::PipelineVertexInputStateCreateInfo()
				.setVertexBindingDescriptions({vibd})
				.setVertexAttributeDescriptions(visds);

			// 入力アセンブリ
			const auto piasci = vk::PipelineInputAssemblyStateCreateInfo()
				.setTopology(vk::PrimitiveTopology::eTriangleList);

			// ビューポート
			const auto viewport = vk::Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
			const auto scissor = vk::Rect2D({0, 0}, {width, height});
			const auto pvsci = vk::PipelineViewportStateCreateInfo()
				.setViewports({viewport})
				.setScissors({scissor});

			// ラスタライゼーション
			const auto prsci = vk::PipelineRasterizationStateCreateInfo()
				.setPolygonMode(vk::PolygonMode::eFill)
				.setCullMode(config.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
				.setFrontFace(vk::FrontFace::eCounterClockwise)
				.setLineWidth(1.0f);

			// マルチサンプル
			const auto pmsci = vk::PipelineMultisampleStateCreateInfo()
				.setRasterizationSamples(vk::SampleCountFlagBits::e1);

			// カラーブレンド
			std::vector<vk::PipelineColorBlendAttachmentState> pcbass;
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
			const auto pcbsci = vk::PipelineColorBlendStateCreateInfo()
				.setAttachments(pcbass);

			// パイプラインレイアウト
			std::vector<vk::DescriptorSetLayout> descSetLayouts;
			for (const auto &n: config.descSets) {
				const auto ci = vk::DescriptorSetLayoutCreateInfo()
					.setBindings(n);
				const auto descSetLayout = device.createDescriptorSetLayout(ci);
				descSetLayouts.push_back(descSetLayout);
				tempDescSetLayouts.push_back(descSetLayout);
			}
			const auto plci = vk::PipelineLayoutCreateInfo()
				.setSetLayouts(descSetLayouts);
			const auto pipelineLayout = device.createPipelineLayout(plci);
			tempPipelineLayout.push_back(pipelineLayout);

			// パイプライン
			cis.push_back(
				vk::GraphicsPipelineCreateInfo()
					.setStages(psscis)
					.setPVertexInputState(&pvisci)
					.setPInputAssemblyState(&piasci)
					.setPViewportState(&pvsci)
					.setPRasterizationState(&prsci)
					.setPMultisampleState(&pmsci)
					.setPColorBlendState(&pcbsci)
					.setLayout(pipelineLayout)
					.setRenderPass(renderPass)
					.setSubpass(config.subpass)
			);

			// IDを記録
			g_pipelineMap.emplace(config.id, g_pipelineMap.size());
		}

		// パイプライン作成
		g_pipelines = device.createGraphicsPipelines(nullptr, cis).value;
		if (g_pipelines.size() != g_pipelineMap.size()) {
			throw;
		}
	} catch (...) {
		return Error::CreateGraphicsPipeline;
	}

	// 一時オブジェクト削除
	for (auto &n: tempPipelineLayout) {
		device.destroyPipelineLayout(n);
	}
	for (auto &n: tempDescSetLayouts) {
		device.destroyDescriptorSetLayout(n);
	}
	for (auto &n: tempShaderModules) {
		device.destroyShaderModule(n);
	}

	return Error::None;
}

void terminate(const vk::Device &device) {
	for (auto &n: g_pipelines) {
		device.destroyPipeline(n);
	}
	g_pipelines.clear();
	g_pipelineMap.clear();
}

} // namespace graphics::pipeline
