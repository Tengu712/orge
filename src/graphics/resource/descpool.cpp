#include "descpool.hpp"

#include "../../config/config.hpp"
#include "../../config/enumconvert.hpp"
#include "../core/core.hpp"

namespace graphics::resource {

std::optional<vk::DescriptorPool> g_descpool;

void initializeDescriptorPool() {
	if (g_descpool) {
		throw "descriptor pool already initialized.";
	}

	// 集計
	uint32_t maxSets = 0;
	std::unordered_map<vk::DescriptorType, uint32_t> sizesMap;
	for (const auto &[_, n]: config::config().pipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<config::DescriptorType, uint32_t> map;
			for (const auto &b: m.bindings) {
				const auto t = config::convertDescriptorType(b.type);
				sizesMap[t] += b.count * m.count;
			}
		}
	}

	// テキストレンダリングパイプライン用のディスクリプタセットを追加
	// TODO: こっちに統合した方が実際に使うパイプラインだけ列挙できるから良さそう？
	// FIXME: というか、現状だと同じパイプラインを複数サブパスで使うことが想定されていない。
	uint32_t textRenderingPipelineCount = 0;
	for (const auto &[_, n]: config::config().renderPasses) {
		for (const auto &m: n.subpasses) {
			for (const auto &o: m.pipelines) {
				if (o == "@text@") {
					textRenderingPipelineCount += 1;
				}
			}
		}
	}
	if (textRenderingPipelineCount > 0) {
		const auto fontCount = config::config().fonts.size();
		maxSets += textRenderingPipelineCount * 2;
		sizesMap[vk::DescriptorType::eStorageBuffer] += textRenderingPipelineCount;
		sizesMap[vk::DescriptorType::eSampledImage] += textRenderingPipelineCount * static_cast<uint32_t>(fontCount);
		sizesMap[vk::DescriptorType::eSampler] += textRenderingPipelineCount;
	}

	// コンピュートパイプライン用のディスクリプタセットを追加
	for (const auto &[_, n]: config::config().computePipelines) {
		for (const auto &m: n.descSets) {
			maxSets += m.count;

			std::unordered_map<config::DescriptorType, uint32_t> map;
			for (const auto &b: m.bindings) {
				const auto t = config::convertComputeDescriptorType(b.type);
				sizesMap[t] += b.count * m.count;
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
		poolSizes.emplace_back(k, v);
	}

	// 作成
	const auto ci = vk::DescriptorPoolCreateInfo()
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setMaxSets(maxSets)
		.setPoolSizes(poolSizes);
	g_descpool = core::device().createDescriptorPool(ci);
}

void destroyDescriptorPool() noexcept {
	if (g_descpool) {
		core::device().destroy(g_descpool.value());
		g_descpool.reset();
	}
}

const vk::DescriptorPool &descpool() {
	if (g_descpool) {
		return g_descpool.value();
	} else {
		throw "descriptor pool not initialized.";
	}
}

} // namespace graphics::resource
