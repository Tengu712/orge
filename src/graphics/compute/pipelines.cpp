#include "pipeline.hpp"

#include "../../asset/asset.hpp"
#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../../error/error.hpp"
#include "../core/core.hpp"
#include "../resource/descpool.hpp"

#include <unordered_map>

namespace graphics::compute {

std::unordered_map<std::string, ComputePipeline> g_pipelines;

void destroyAllComputePipelines() {
	g_pipelines.clear();
}

void initializeComputePipelines() {
	if (config::config().computePipelines.empty()) {
		return;
	}

	std::vector<std::string_view> ids;
	std::vector<vk::UniqueShaderModule> shaders;
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	std::vector<std::vector<vk::UniqueDescriptorSetLayout>> descSetLayoutss;
	std::vector<std::vector<std::vector<vk::UniqueDescriptorSet>>> descSetsss;
	std::vector<vk::UniquePipelineLayout> pipelineLayouts;
	std::vector<vk::ComputePipelineCreateInfo> cis;

	const auto size = config::config().computePipelines.size();
	ids.reserve(size);
	shaders.reserve(size);
	shaderStages.reserve(size);
	descSetLayoutss.reserve(size);
	descSetsss.reserve(size);
	pipelineLayouts.reserve(size);
	cis.reserve(size);

	for (const auto &[id, n]: config::config().computePipelines) {
		// id
		ids.emplace_back(id);

		// シェーダモジュール
		const auto aid = error::at(config::config().assetMap, n.shader, "assets");
		const auto raw = asset::getAsset(aid);
		const auto code = std::vector<uint32_t>(
			reinterpret_cast<const uint32_t *>(raw.data()),
			reinterpret_cast<const uint32_t *>(raw.data() + raw.size())
		);
		const auto shaderCi = vk::ShaderModuleCreateInfo()
			.setCode(code);
		auto shader = core::device().createShaderModuleUnique(shaderCi);
		shaders.push_back(std::move(shader));

		// シェーダステージ
		shaderStages.push_back(
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eCompute)
				.setModule(shaders.back().get())
				.setPName("main")
		);

		// ディスクリプタセットレイアウト
		std::vector<vk::UniqueDescriptorSetLayout> descSetLayouts;
		descSetLayouts.reserve(n.descSets.size());
		for (const auto &m: n.descSets) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;
			for (const auto &o: m.bindings) {
				bindings.emplace_back(
					static_cast<uint32_t>(bindings.size()),
					config::convertComputeDescriptorType(o.type),
					o.count,
					vk::ShaderStageFlagBits::eCompute,
					nullptr
				);
			}
			const auto ci = vk::DescriptorSetLayoutCreateInfo()
				.setBindings(bindings);
			auto descSetLayout = core::device().createDescriptorSetLayoutUnique(ci);
			descSetLayouts.push_back(std::move(descSetLayout));
		}
		descSetLayoutss.push_back(std::move(descSetLayouts));

		// ディスクリプタセット確保
		std::vector<std::vector<vk::UniqueDescriptorSet>> descSetss;
		descSetss.reserve(n.descSets.size());
		for (size_t i = 0; i < n.descSets.size(); ++i) {
			std::vector<vk::DescriptorSetLayout> layouts;
			for (size_t j = 0; j < n.descSets[i].count; ++j) {
				layouts.push_back(descSetLayoutss.back()[i].get());
			}
			const auto ai = vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(resource::descpool())
				.setSetLayouts(layouts);
			auto descSets = core::device().allocateDescriptorSetsUnique(ai);
			descSetss.push_back(std::move(descSets));
		}
		descSetsss.push_back(std::move(descSetss));

		// パイプラインレイアウト
		std::vector<vk::DescriptorSetLayout> rawDescSetLayouts;
		for (const auto& n: descSetLayoutss.back()) {
			rawDescSetLayouts.push_back(n.get());
		}
		const auto plci = vk::PipelineLayoutCreateInfo()
			.setSetLayouts(rawDescSetLayouts);
		auto pipelineLayout = core::device().createPipelineLayoutUnique(plci);
		pipelineLayouts.push_back(std::move(pipelineLayout));

		// 作成情報
		cis.emplace_back(vk::PipelineCreateFlags{}, shaderStages.back(), pipelineLayouts.back().get());
	}

	auto pipelines = core::device().createComputePipelinesUnique({}, cis).value;
	if (pipelines.size() != config::config().computePipelines.size()) {
		throw "failed to create compute pipelines.";
	}

	for (size_t i = 0; i < pipelines.size(); ++i) {
		const std::string id(ids[i]);
		g_pipelines.try_emplace(
			id,
			id,
			std::move(descSetLayoutss[i]),
			std::move(descSetsss[i]),
			std::move(pipelineLayouts[i]),
			std::move(pipelines[i])
		);
	}
}

ComputePipeline &getComputePipeline(const std::string &id) {
	return error::atMut(g_pipelines, id, "compute pipelines");
}

} // namespace graphics::compute
