#pragma once

#include <orge/orge.h>

#include "../graphics/renderer/renderer.hpp"

namespace api {

inline void beginRender() {
	graphics::renderer::renderer().begin();
}

inline void endRender() {
	graphics::renderer::renderer().end();
}

inline void bindMesh(OrgeBindMeshParam *param) {
	graphics::renderer::renderer().getContext().bindMesh(param->meshId);
}

inline void beginRenderPass(OrgeBeginRenderPassParam *param) {
	graphics::renderer::renderer().getContext().beginRenderPass(param->renderPassId);
}

inline void endRenderPass() {
	graphics::renderer::renderer().getContext().endRenderPass();
}

inline void nextSubpass() {
	graphics::renderer::renderer().getContext().nextSubpass();
}

inline void bindPipeline(OrgeBindPipelineParam *param) {
	graphics::renderer::renderer().getContext().bindPipeline(param->pipelineId, param->indices);
}

inline void draw(OrgeDrawParam *param) {
	graphics::renderer::renderer().getContext().draw(param->instanceCount, param->instanceOffset);
}

inline void drawDirectly(OrgeDrawDirectlyParam *param) {
	graphics::renderer::renderer()
		.getContext()
		.drawDirectly(param->vertexCount, param->instanceCount, param->instanceOffset);
}

} // namespace api
