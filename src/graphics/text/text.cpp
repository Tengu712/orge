#include "text.hpp"

#include "../../config/config.hpp"
#include "../../error/error.hpp"
#include "../resource/buffer.hpp"
#include "../resource/charatlus.hpp"
#include "../resource/sampler.hpp"

#include <orge.h>

namespace graphics::text {

struct TextRenderingInstance {
	alignas(16) float transform[16];
	alignas(16) float uv[4];
	alignas(16) uint32_t texId[4];
};

std::string formatBufferId(const std::string &renderPassId, uint32_t subpassIndex, const std::string &pipelineId) {
	return std::format("@buffer-tr@:{}:{}:{}", renderPassId, subpassIndex, pipelineId);
}

void destroyTextRenderingResources() {
	resource::destroyAllCharAtluses();
}

void initializeTextRenderingResources() {
	resource::initializeAllCharAtluses();
	resource::addSampler("@sampler-tr@", true, true, false);
	for (const auto &[renderPassId, n]: config::config().renderPasses) {
		for (size_t subpassIndex = 0; subpassIndex < n.subpasses.size(); ++subpassIndex) {
			for (const auto &pipelineId: n.subpasses[subpassIndex].pipelines) {
				const auto &m = config::config().pipelines.at(pipelineId);
				if (!m.textRendering) {
					continue;
				}
				resource::addBuffer(
					formatBufferId(renderPassId, subpassIndex, pipelineId),
					sizeof(TextRenderingInstance) * m.charCount,
					true
				);
			}
		}
	}
}

void rasterizeText(const std::string &fontId, const std::string &text) {
	resource::getCharAtlus(fontId).rasterizeCharacters(text);
}

/*
void putText(
	const std::string &pipelineId,
	const std::string &fontId,
	const std::string &text,
	float x,
	float y,
	float height,
	OrgeTextLocationHorizontal horizontal,
	OrgeTextLocationVertical vertical
) {
	// 必要な情報を取得
	auto &charAtlus = error::atMut(_charAtluss, fontId, "fonts");
	const auto texId = error::at(config::config().fontMap, fontId, "fonts");
	const auto meshSize = charAtlus.calcMeshSize(height);
	const auto ru = charAtlus.getRangeOfU();
	const auto rv = charAtlus.getRangeOfV();
	const auto extentW = static_cast<float>(config::config().width);
	const auto extentH = static_cast<float>(config::config().height);
	const auto offset = _textOffset.contains(pipelineId) ? _textOffset[pipelineId] : 0;
	const auto charCount = static_cast<size_t>(error::at(_charCounts, pipelineId, "pipelines"));

	// すべてのメッシュを左上原点にする
	x += meshSize / 2.0f;
	y += meshSize / 2.0f;

	// Y座標をベースラインに移す
	y += charAtlus.calcAscent(height);

	// 準備
	struct LineInfo {
		size_t startIndex;
		size_t endIndex;
		float minX;
		float maxX;
	};
	std::vector<TextRenderingInstance> instances;
	std::vector<LineInfo> lines;
	lines.push_back({0, 0, x, x});
	const auto startX = x;

	// すべての文字に対してとりあえず構築
	auto itr = text.begin();
	auto end = text.end();
	while (itr != end && offset + instances.size() < charCount) {
		const auto codepoint = static_cast<uint32_t>(utf8::next(itr, end));

		// LFは改行
		if (codepoint == 10) {
			x = startX;
			y += charAtlus.calcLineAdvance(height);
			lines.push_back({instances.size(), instances.size(), x, x});
			continue;
		}
		// CRはスキップ
		if (codepoint == 13) {
			continue;
		}

		const auto c = charAtlus.getScaledCharacter(codepoint, height);

		// NOTE: 文字が存在しない場合はスキップする。
		if (!c) {
			continue;
		}

		instances.push_back({});
		auto &n = instances.back();
		std::fill_n(n.transform, 16, 0.0f);
		std::fill_n(n.texId, 4, 0);
		n.transform[0] = meshSize;
		n.transform[5] = meshSize;
		n.transform[10] = 1.0f;
		n.transform[12] = std::round(x + c->ox);
		n.transform[13] = std::round(y + c->oy);
		n.transform[15] = 1.0f;
		n.uv[0] = c->u;
		n.uv[1] = c->v;
		n.uv[2] = ru;
		n.uv[3] = rv;
		n.texId[0] = texId;
		x += c->advance;

		// 行情報を更新
		auto &l = lines.back();
		l.endIndex = instances.size() - 1;
		l.minX = std::min(l.minX, n.transform[12]);
		l.maxX = std::max(l.maxX, n.transform[12] + c->w);
	}

	// 描画すべき文字がないなら終了
	if (instances.empty()) {
		return;
	}

	// 文字列全体の高さを計算
	const auto entireHeight = meshSize + charAtlus.calcLineAdvance(height) * static_cast<uint32_t>(lines.size() - 1);

	// 座標修正
	for (const auto &l: lines) {
		// 空白行をスキップ
		if (l.startIndex >= instances.size()) {
			break;
		}

		const auto lineWidth = l.maxX - l.minX;

		for (size_t i = l.startIndex; i < l.endIndex + 1; ++i) {
			auto &n = instances[i];

			// 各行ごとにX座標を修正
			switch (horizontal) {
			case ORGE_TEXT_LOCATION_HORIZONTAL_LEFT:
				break;
			case ORGE_TEXT_LOCATION_HORIZONTAL_CENTER:
				n.transform[12] -= lineWidth / 2.0f;
				break;
			case ORGE_TEXT_LOCATION_HORIZONTAL_RIGHT:
				n.transform[12] -= lineWidth;
				break;
			}

			// Y座標を修正
			switch (vertical) {
			case ORGE_TEXT_LOCATION_VERTICAL_TOP:
				break;
			case ORGE_TEXT_LOCATION_VERTICAL_MIDDLE:
				n.transform[13] -= entireHeight / 2.0f;
				break;
			case ORGE_TEXT_LOCATION_VERTICAL_BOTTOM:
				n.transform[13] -= entireHeight;
				break;
			}

			// XY座標を整数値に
			n.transform[12] = std::round(n.transform[12]);
			n.transform[13] = std::round(n.transform[13]);

			// クリッピング座標系へ
			n.transform[0] /= extentW;
			n.transform[5] /= extentH;
			n.transform[12] -= extentW / 2.0f;
			n.transform[12] /= extentW / 2.0f;
			n.transform[13] -= extentH / 2.0f;
			n.transform[13] /= extentH / 2.0f;
		}
	}

	// アップロード
	error::at(_buffers, "@buffer@" + pipelineId, "buffers")
		.update(
			_device,
			instances.data(),
			instances.size() * sizeof(TextRenderingInstance),
			_textOffset.contains(pipelineId) ? _textOffset[pipelineId] : 0
		);

	// offsetを進める
	if (_textOffset.contains(pipelineId)) {
		_textOffset[pipelineId] += instances.size();
	} else {
		_textOffset.emplace(pipelineId, instances.size());
	}
}
*/

} // namespace graphics::text
