#include "text.hpp"

#include "../../config/config.hpp"
#include "../resource/buffer.hpp"
#include "../resource/charatlus.hpp"
#include "../resource/sampler.hpp"

namespace graphics::text {

void destroyTextRenderingResources() {
	resource::destroyAllCharAtluses();
}

void initializeTextRenderingResources() {
	// テキストレンダリングパイプラインを使わないなら不要
	bool need = false;
	for (const auto &[_, n]: config::config().renderPasses) {
		for (const auto &m: n.subpasses) {
			if (m.pipelines.contains("@text@")) {
				need = true;
				break;
			}
		}
	}
	if (!need) {
		return;
	}

	resource::initializeAllCharAtluses();
	resource::addSampler("@sampler-tr@", true, true, false);
	resource::addBuffer("@buffer-tr@", sizeof(TextRenderingInstance) * config::config().charCount, true);
}

void rasterizeText(const std::string &fontId, const std::string &text) {
	resource::getCharAtlus(fontId).rasterizeCharacters(text);
}

} // namespace graphics::text
