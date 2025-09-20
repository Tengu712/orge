#pragma once

#include <orge/orge.h>

#include "../graphics/renderer/renderer.hpp"

namespace api {

inline void bindComputePipeline(OrgeBindComputePipelineParam *param) {
	graphics::renderer::renderer().getContext().bindComputePipeline(param->pipelineId, param->indices);
}

inline void dispatch(OrgeDispatchParam *param) {
	graphics::renderer::renderer().getContext().dispatch(param->x, param->y, param->z);
}

} // namespace api
