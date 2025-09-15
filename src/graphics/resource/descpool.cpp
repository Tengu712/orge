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
	std::unordered_map<config::DescriptorType, uint32_t> sizesMap;
	for (const auto &[_, n]: config::config().pipelines) {
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

	// テキストレンダリングパイプライン用のディスクリプタセットを追加
	// TODO: こっちに統合した方が実際に使うパイプラインだけ列挙できるから良さそう？
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
		sizesMap[config::DescriptorType::StorageBuffer] += textRenderingPipelineCount;
		sizesMap[config::DescriptorType::Texture] += textRenderingPipelineCount * fontCount;
		sizesMap[config::DescriptorType::Sampler] += textRenderingPipelineCount;
	}

	// ディスクリプタセットが不要ならディスクリプタプールも不要
	if (maxSets == 0) {
		return;
	}

	// マップからベクタへ
	std::vector<vk::DescriptorPoolSize> poolSizes;
	for (const auto &[k, v]: sizesMap) {
		poolSizes.emplace_back(config::convertDescriptorType(k), v);
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
