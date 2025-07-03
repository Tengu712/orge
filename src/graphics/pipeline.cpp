#include "pipeline.hpp"

#include "swapchain.hpp"

#include <fstream>
#include <ranges>
#include <string>
#include <unordered_map>

namespace graphics::pipeline {

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
	// 入力アセンブリ
	vk::PipelineInputAssemblyStateCreateInfo piasci;
	// ビューポート
	std::vector<vk::Viewport> viewports;
	std::vector<vk::Rect2D> scissors;
	vk::PipelineViewportStateCreateInfo pvsci;
	// ラスタライゼーション
	vk::PipelineRasterizationStateCreateInfo prsci;
	// マルチサンプル
	vk::PipelineMultisampleStateCreateInfo pmsci;
	// カラーブレンド
	std::vector<vk::PipelineColorBlendAttachmentState> pcbass;
	vk::PipelineColorBlendStateCreateInfo pcbsci;
	// ディスクリプタセットレイアウト
	std::vector<vk::DescriptorSetLayout> descSetLayouts;
	// パイプラインレイアウト
	vk::PipelineLayout pipelineLayout;
	// サブパスID
	uint32_t subpass;
};

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

Error createPipelineCreateTempInfo(const Pipeline &config, const vk::Device &device, std::vector<PipelineCreateTempInfo> &cits, uint32_t width, uint32_t height) {
	cits.push_back({});
	auto &cit = cits.back();

	// シェーダステージ
	cit.vertexShader = createShaderModule(device, config.vertexShader);
	cit.fragmentShader = createShaderModule(device, config.fragmentShader);
	if (!cit.vertexShader || !cit.fragmentShader) {
		return Error::CreateShaderModule;
	}
	cit.psscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eVertex,
		cit.vertexShader,
		"main"
	);
	cit.psscis.emplace_back(
		vk::PipelineShaderStageCreateFlags(),
		vk::ShaderStageFlagBits::eFragment,
		cit.fragmentShader,
		"main"
	);

	// 頂点入力
	uint32_t sum = 0;
	for (uint32_t i = 0; i < config.vertexInputAttributes.size(); ++i) {
		const auto &n = config.vertexInputAttributes.at(i);
		cit.viads.emplace_back(
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
	cit.vibds.emplace_back(0, sizeof(float) * sum, vk::VertexInputRate::eVertex);
	cit.pvisci = vk::PipelineVertexInputStateCreateInfo()
		.setVertexBindingDescriptions(cit.vibds)
		.setVertexAttributeDescriptions(cit.viads);

	// 入力アセンブリ
	cit.piasci = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	// ビューポート
	cit.viewports.emplace_back(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f);
	cit.scissors.emplace_back(vk::Offset2D(0, 0), vk::Extent2D(width, height));
	cit.pvsci = vk::PipelineViewportStateCreateInfo()
		.setViewports(cit.viewports)
		.setScissors(cit.scissors);

	// ラスタライゼーション
	cit.prsci = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(config.culling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone)
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setLineWidth(1.0f);

	// マルチサンプル
	cit.pmsci = vk::PipelineMultisampleStateCreateInfo()
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	// カラーブレンド
	for (const auto &n: config.colorBlends) {
		cit.pcbass.emplace_back(
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
	cit.pcbsci = vk::PipelineColorBlendStateCreateInfo()
		.setAttachments(cit.pcbass);

	// ディスクリプタセットレイアウト
	for (const auto &n: config.descSets) {
		const auto ci = vk::DescriptorSetLayoutCreateInfo()
			.setBindings(n);
		try {
			cit.descSetLayouts.push_back(device.createDescriptorSetLayout(ci));
		} catch (...) {
			return Error::CreateGraphicsPipeline;
		}
	}

	// パイプラインレイアウト
	const auto plci = vk::PipelineLayoutCreateInfo()
		.setSetLayouts(cit.descSetLayouts);
	try {
		cit.pipelineLayout = device.createPipelineLayout(plci);
	} catch (...) {
		return Error::CreateGraphicsPipeline;
	}

	// サブパスID
	cit.subpass = config.subpass;

	// 終了
	cits.push_back(cit);
	return Error::None;
}

Error initialize(const Config &config, const vk::Device &device, const vk::RenderPass &renderPass) {
	if (config.pipelines.empty()) {
		return Error::None;
	}

	const auto imageSize = swapchain::getImageSize();
	const auto width = imageSize.width;
	const auto height = imageSize.height;

	std::vector<PipelineCreateTempInfo> cits;
	cits.reserve(config.pipelines.size());
	for (const auto &n: config.pipelines) {
		g_pipelineMap.emplace(n.id, g_pipelineMap.size());
		CHECK(createPipelineCreateTempInfo(n, device, cits, width, height));
	}

	std::vector<vk::GraphicsPipelineCreateInfo> cis;
	for (const auto &n: cits) {
		cis.push_back(
			vk::GraphicsPipelineCreateInfo()
				.setStages(n.psscis)
				.setPVertexInputState(&n.pvisci)
				.setPInputAssemblyState(&n.piasci)
				.setPViewportState(&n.pvsci)
				.setPRasterizationState(&n.prsci)
				.setPMultisampleState(&n.pmsci)
				.setPColorBlendState(&n.pcbsci)
				.setLayout(n.pipelineLayout)
				.setRenderPass(renderPass)
				.setSubpass(n.subpass)
		);
	}
	try {
		g_pipelines = device.createGraphicsPipelines(nullptr, cis).value;
	} catch (...) {
		return Error::CreateGraphicsPipeline;
	}

	if (g_pipelines.size() != g_pipelineMap.size()) {
		return Error::CreateGraphicsPipeline;
	}

	// 一時オブジェクト削除
	for (auto &n: cits) {
		device.destroyPipelineLayout(n.pipelineLayout);
		for (auto &m: n.descSetLayouts) {
			device.destroyDescriptorSetLayout(m);
		}
		device.destroyShaderModule(n.fragmentShader);
		device.destroyShaderModule(n.vertexShader);
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
