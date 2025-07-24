#include "pipeline.hpp"

#include <fstream>

namespace graphics {

struct PipelineCreateTemporaryInfos {
	// シェーダステージ
	vk::ShaderModule vertexShader;
	vk::ShaderModule fragmentShader;
	std::vector<vk::PipelineShaderStageCreateInfo> sscis;

	// 頂点入力
	std::vector<vk::VertexInputAttributeDescription> viads;
	std::vector<vk::VertexInputBindingDescription> vibds;
	vk::PipelineVertexInputStateCreateInfo visci;

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

	// インプットアタッチメント
	std::vector<InputAttachment> inputs;
};


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

vk::Viewport adjustViewport(uint32_t ow, uint32_t oh, const vk::Extent2D &extent) {
	float o = static_cast<float>(ow)           / static_cast<float>(oh);
	float n = static_cast<float>(extent.width) / static_cast<float>(extent.height);
	if (n > o) {
		const auto h = static_cast<float>(extent.height);
		const auto w = h * o;
		const auto x = (extent.width - w) / 2.0f;
		return vk::Viewport(x, 0.0f, w, h, 0.0f, 1.0f);
	} else {
		const auto w = static_cast<float>(extent.width);
		const auto h = w / o;
		const auto y = (extent.height - h) / 2.0f;
		return vk::Viewport(0.0f, y, w, h, 0.0f, 1.0f);
	}
}

std::unordered_map<std::string, Pipeline> createPipelines(
	const config::Config &config,
	const vk::Device &device,
	const vk::RenderPass &renderPass,
	const vk::DescriptorPool &descPool,
	const vk::Extent2D &extent
) {
	if (config.pipelines.empty()) {
		return {};
	}

	// 入力アセンブリ
	const auto iasci = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	// ビューポート
	std::vector<vk::Viewport> viewports;
	viewports.emplace_back(adjustViewport(config.width, config.height, extent));
	std::vector<vk::Rect2D> scissors;
	scissors.emplace_back(vk::Offset2D(0, 0), extent);
	const auto vsci = vk::PipelineViewportStateCreateInfo()
		.setViewports(viewports)
		.setScissors(scissors);

	// マルチサンプル
	const auto msci = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	// 作成情報作成
	std::vector<PipelineCreateTemporaryInfos> ctis;
	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	ctis.reserve(config.pipelines.size());
	cis.reserve(config.pipelines.size());
	for (const auto &n: config.pipelines) {
		ctis.push_back({});
		auto &cti = ctis.back();

		// シェーダステージ
		cti.vertexShader = createShaderModule(device, n.vertexShader);
		cti.fragmentShader = createShaderModule(device, n.fragmentShader);
		if (!cti.vertexShader || !cti.fragmentShader) {
			throw "failed to create shader modules.";
		}
		cti.sscis.emplace_back(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			cti.vertexShader,
			"main"
		);
		cti.sscis.emplace_back(
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			cti.fragmentShader,
			"main"
		);

		// 頂点入力
		uint32_t sum = 0;
		for (size_t i = 0; i < n.vertexInputAttributes.size(); ++i) {
			const auto &m = n.vertexInputAttributes[i];
			cti.viads.emplace_back(
				static_cast<uint32_t>(i),
				0,
				m == 1 ? vk::Format::eR32Sfloat
				: m == 2 ? vk::Format::eR32G32Sfloat
				: m == 3 ? vk::Format::eR32G32B32Sfloat
				: vk::Format::eR32G32B32A32Sfloat,
				static_cast<uint32_t>(sizeof(float)) * sum
			);
			sum += m;
		}
		cti.vibds.emplace_back(0, static_cast<uint32_t>(sizeof(float) * sum), vk::VertexInputRate::eVertex);
		cti.visci = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptions(cti.vibds)
			.setVertexAttributeDescriptions(cti.viads);

		// ラスタライゼーション
		cti.rsci = vk::PipelineRasterizationStateCreateInfo()
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(n.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
			.setFrontFace(vk::FrontFace::eCounterClockwise)
			.setLineWidth(1.0f);

		// デプスステンシル
		cti.dssci = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(n.depthTest)
			.setDepthWriteEnable(n.depthTest)
			.setDepthCompareOp(vk::CompareOp::eLess)
			.setMaxDepthBounds(1.0f);

		// カラーブレンド
		for (const auto &m: n.colorBlends) {
			cti.cbass.emplace_back(
				m ? vk::True : vk::False,
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
		cti.cbsci = vk::PipelineColorBlendStateCreateInfo()
			.setAttachments(cti.cbass);

		// ディスクリプタセットレイアウト
		for (const auto &m: n.descSets) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			for (const auto &o: m.bindings) {
				bindings.emplace_back(
					static_cast<uint32_t>(bindings.size()),
					o.type == config::DescriptorType::Texture
						? vk::DescriptorType::eSampledImage
						: o.type == config::DescriptorType::Sampler
						? vk::DescriptorType::eSampler
						: o.type == config::DescriptorType::UniformBuffer
						? vk::DescriptorType::eUniformBuffer
						: o.type == config::DescriptorType::StorageBuffer
						? vk::DescriptorType::eStorageBuffer
						: o.type == config::DescriptorType::InputAttachment
						? vk::DescriptorType::eInputAttachment
						: throw,
					o.count,
					o.stage == config::ShaderStages::Vertex
						? vk::ShaderStageFlagBits::eVertex
						: o.stage == config::ShaderStages::Fragment
						? vk::ShaderStageFlagBits::eFragment
						: o.stage == config::ShaderStages::VertexAndFragment
						? vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
						: throw,
					nullptr
				);
			}
			const auto ci = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(bindings);
			cti.descSetLayouts.push_back(device.createDescriptorSetLayout(ci));
		}

		// パイプラインレイアウト
		const auto plci = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(cti.descSetLayouts);
		cti.pipelineLayout = device.createPipelineLayout(plci);

		// サブパスID
		if (!config.subpassMap.contains(n.subpass)) {
			throw std::format("subpass '{}' is not defined.", n.subpass);
		}
		cti.subpass = config.subpassMap.at(n.subpass);

		// ディスクリプタセット確保
		for (size_t i = 0; i < n.descSets.size(); ++i) {
			std::vector<vk::DescriptorSetLayout> layouts;
			for (size_t j = 0; j < n.descSets[i].count; ++j) {
				layouts.push_back(cti.descSetLayouts[i]);
			}
			const auto ai = vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(descPool)
				.setSetLayouts(layouts);
			cti.descSets.push_back(device.allocateDescriptorSets(ai));
		}

		// インプットアタッチメント
		for (size_t i = 0; i < n.descSets.size(); ++i) {
			for (size_t j = 0; j < n.descSets[i].bindings.size(); ++j) {
				if (n.descSets[i].bindings[j].attachment == "") {
					continue;
				}
				const auto isTexture = n.descSets[i].bindings[j].type == config::DescriptorType::Texture;
				const auto isAttachment = n.descSets[i].bindings[j].type == config::DescriptorType::InputAttachment;
				if (!isTexture && !isAttachment) {
					throw;
				}
				cti.inputs.emplace_back(
					config.attachmentMap.at(n.descSets[i].bindings[j].attachment),
					static_cast<uint32_t>(i),
					static_cast<uint32_t>(j),
					isTexture
				);
			}
		}

		// 作成情報
		cis.push_back(
			vk::GraphicsPipelineCreateInfo()
				.setStages(cti.sscis)
				.setPVertexInputState(&cti.visci)
				.setPInputAssemblyState(&iasci)
				.setPViewportState(&vsci)
				.setPRasterizationState(&cti.rsci)
				.setPMultisampleState(&msci)
				.setPDepthStencilState(&cti.dssci)
				.setPColorBlendState(&cti.cbsci)
				.setLayout(cti.pipelineLayout)
				.setRenderPass(renderPass)
				.setSubpass(cti.subpass)
		);
	}

	// パイプライン作成
	const auto ps = device.createGraphicsPipelines(nullptr, cis).value;

	// 作成
	std::unordered_map<std::string, Pipeline> pipelines;
	pipelines.reserve(ps.size());
	for (size_t i = 0; i < ps.size(); ++i) {
		pipelines.emplace(
			config.pipelines[i].id,
			Pipeline {
				ps[i],
				ctis[i].pipelineLayout,
				std::move(ctis[i].descSetLayouts),
				std::move(ctis[i].descSets),
				std::move(ctis[i].inputs),
			}
		);
	}

	// 終了
	for (auto &n: ctis) {
		device.destroyShaderModule(n.fragmentShader);
		device.destroyShaderModule(n.vertexShader);
	}

	return pipelines;
}

} // namespace graphics
