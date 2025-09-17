#include <orge.h>

#include "graphics/compute/pipeline.hpp"
#include "graphics/renderer/renderer.hpp"
#include "orge-private.hpp"

#define DEFINE_UPDATE_DESC_FUNC(n) \
	uint8_t orgeUpdateCompute##n##Descriptor( \
		const char *pipelineId, \
		const char *id, \
		uint32_t set, \
		uint32_t index, \
		uint32_t binding, \
		uint32_t offset \
	) { \
		TRY( \
			graphics::compute::getComputePipeline(pipelineId) \
				.update##n##Descriptor(id, set, index, binding, offset) \
		); \
	}

DEFINE_UPDATE_DESC_FUNC(Buffer)
DEFINE_UPDATE_DESC_FUNC(Image)
DEFINE_UPDATE_DESC_FUNC(Sampler)

#define TRY_OR(n) \
	bool result = false; \
	CHECK(n); \
	if (!result) graphics::renderer::renderer().reset(); \
	return static_cast<uint8_t>(result);

uint8_t orgeBindComputePipeline(const char *pipelineId, uint32_t const *indices) {
	TRY_OR(graphics::renderer::renderer().getContext().bindComputePipeline(pipelineId, indices));
}

uint8_t orgeDispatch(uint32_t x, uint32_t y, uint32_t z) {
	TRY_OR(graphics::renderer::renderer().getContext().dispatch(x, y, z));
}
