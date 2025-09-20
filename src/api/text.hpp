#pragma once

#include <orge/orge.h>

#include "../graphics/text/text.hpp"
#include "../graphics/renderer/renderer.hpp"

namespace api {

inline void rasterizeCharacters(OrgeRasterizeCharactersParam *param) {
	graphics::text::rasterizeText(param->id, param->s);
}

inline void layoutText(OrgeLayoutTextParam *param) {
	graphics::text::layoutText(
		param->renderPassId,
		param->subpassId,
		param->fontId,
		param->text,
		param->x,
		param->y,
		param->height,
		static_cast<OrgeTextLocationHorizontal>(param->horizontal),
		static_cast<OrgeTextLocationVertical>(param->vertical)
	);
}

inline void drawTexts() {
	graphics::renderer::renderer().getContext().drawTexts();
}

} // namespace api
