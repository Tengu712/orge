#include "pipeline.hpp"

// TODO: ちょっと気持ち悪い。
#include "../swapchain/swapchain.hpp"
// TODO: ちょっと気持ち悪い。
#include "../framebuffer/framebuffer.hpp"
#include "buffer/buffer.hpp"
#include "image/image.hpp"
#include "sampler/sampler.hpp"

#include <fstream>

namespace graphics::rendering::pipeline {

struct InputAttachment {
	const uint32_t id;
	const uint32_t set;
	const uint32_t binding;
	const bool isTexture;

	InputAttachment() = delete;
	InputAttachment(uint32_t id, uint32_t set, uint32_t binding, bool isTexture):
		id(id), set(set), binding(binding), isTexture(isTexture)
	{}
};

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

struct Pipeline {
	const vk::Pipeline pipeline;
	const vk::PipelineLayout pipelineLayout;
	const std::vector<vk::DescriptorSetLayout> descSetLayouts;
	const std::vector<std::vector<vk::DescriptorSet>> descSets;
	const std::vector<InputAttachment> inputs;
};

vk::DescriptorPool g_descPool;
std::unordered_map<std::string, Pipeline> g_pipelines;

void terminate(const vk::Device &device) {
	sampler::terminate(device);
	image::terminate(device);
	buffer::terminate(device);

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

void createDescriptorPool(const config::Config &config, const vk::Device &device) {
	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<config::DescriptorType, uint32_t> sizesMap;
	for (const auto &n: config.pipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<config::DescriptorType, uint32_t> map;
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
			k == config::DescriptorType::Texture
				? vk::DescriptorType::eSampledImage
				: k == config::DescriptorType::Sampler
				? vk::DescriptorType::eSampler
				: k == config::DescriptorType::UniformBuffer
				? vk::DescriptorType::eUniformBuffer
				: k == config::DescriptorType::StorageBuffer
				? vk::DescriptorType::eStorageBuffer
				: k == config::DescriptorType::InputAttachment
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

void createPipelines(const config::Config &config, const vk::Device &device, const vk::RenderPass &renderPass) {
	// 入力アセンブリ
	const auto iasci = vk::PipelineInputAssemblyStateCreateInfo()
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	// ビューポート
	const auto imageSize = swapchain::getImageSize();
	std::vector<vk::Viewport> viewports;
	viewports.emplace_back(0.0f, 0.0f, static_cast<float>(imageSize.width), static_cast<float>(imageSize.height), 0.0f, 1.0f);
	std::vector<vk::Rect2D> scissors;
	scissors.emplace_back(vk::Offset2D(0, 0), vk::Extent2D(imageSize.width, imageSize.height));
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
			const auto &m = n.vertexInputAttributes.at(i);
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
				.setDescriptorPool(g_descPool)
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
	const auto pipelines = device.createGraphicsPipelines(nullptr, cis).value;

	// 作成
	for (size_t i = 0; i < pipelines.size(); ++i) {
		g_pipelines.emplace(
			config.pipelines[i].id,
			Pipeline {
				pipelines[i],
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
}

void initialize(
	const config::Config &config,
	const vk::Device &device,
	const vk::CommandPool &commandPool,
	const vk::RenderPass &renderPass
) {
	if (config.pipelines.empty()) {
		return;
	}
	createDescriptorPool(config, device);
	createPipelines(config, device, renderPass);

	image::initialize(device, commandPool);
}

void bind(const vk::Device &device, const vk::CommandBuffer &commandBuffer, uint32_t index, const char *id) {
	const auto &pipeline = g_pipelines.at(id);

	// パイプラインをバインド
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.pipeline);

	// すべてのインプットアタッチメントを更新
	// NOTE: アタッチメントはスワップチェーンイメージの枚数分作られるため、
	//       レンダーパスコマンドごとに更新しなければならない。
	for (const auto &n: pipeline.inputs) {
		const auto &attachment = framebuffer::getAttachment(n.id, index);
		const auto ii = vk::DescriptorImageInfo(nullptr, attachment.view, vk::ImageLayout::eShaderReadOnlyOptimal);
		const auto &descSets = pipeline.descSets.at(n.set);
		std::vector<vk::WriteDescriptorSet> wdss;
		for (size_t i = 0; i < descSets.size(); ++i) {
			wdss.push_back(
				vk::WriteDescriptorSet()
					.setDstSet(descSets.at(i))
					.setDstBinding(n.binding)
					.setDescriptorCount(1)
					.setDescriptorType(n.isTexture ? vk::DescriptorType::eSampledImage : vk::DescriptorType::eInputAttachment)
					.setImageInfo(ii)
			);
		}
		device.updateDescriptorSets(static_cast<uint32_t>(descSets.size()), wdss.data(), 0, nullptr);
	}
}

void bindDescriptorSets(const vk::CommandBuffer &commandBuffer, const char *id, uint32_t const *indices) {
	const auto &pipeline = g_pipelines.at(id);

	std::vector<vk::DescriptorSet> sets;
	for (size_t i = 0; i < pipeline.descSets.size(); ++i) {
		sets.push_back(pipeline.descSets.at(i).at(indices[i]));
	}

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipeline.pipelineLayout,
		0,
		static_cast<uint32_t>(sets.size()),
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
	uint32_t binding,
	uint32_t offset
) {
	const auto &buffer = buffer::get(bufferId);
	const auto bi = vk::DescriptorBufferInfo(buffer.buffer, 0, vk::WholeSize);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(g_pipelines.at(pipelineId).descSets.at(set).at(index))
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(buffer.isStorage ? vk::DescriptorType::eStorageBuffer : vk::DescriptorType::eUniformBuffer)
		.setBufferInfo(bi);
	device.updateDescriptorSets(1, &ds, 0, nullptr);
}

void updateImageDescriptor(
	const vk::Device &device,
	const char *imageId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	const auto &image = image::get(imageId);
	const auto ii = vk::DescriptorImageInfo(nullptr, image.view, vk::ImageLayout::eShaderReadOnlyOptimal);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(g_pipelines.at(pipelineId).descSets.at(set).at(index))
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampledImage)
		.setImageInfo(ii);
	device.updateDescriptorSets(1, &ds, 0, nullptr);
}

void updateSamplerDescriptor(
	const vk::Device &device,
	const char *samplerId,
	const char *pipelineId,
	uint32_t set,
	uint32_t index,
	uint32_t binding,
	uint32_t offset
) {
	const auto &sampler = sampler::get(samplerId);
	const auto ii = vk::DescriptorImageInfo(sampler, nullptr, vk::ImageLayout::eShaderReadOnlyOptimal);
	const auto ds = vk::WriteDescriptorSet()
		.setDstSet(g_pipelines.at(pipelineId).descSets.at(set).at(index))
		.setDstBinding(binding)
		.setDstArrayElement(offset)
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eSampler)
		.setImageInfo(ii);
	device.updateDescriptorSets(1, &ds, 0, nullptr);
}

} // namespace graphics::rendering::pipeline
