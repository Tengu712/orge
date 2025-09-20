#pragma once

#include <orge/orge.h>

#include "../graphics/renderer/renderer.hpp"
#include "../graphics/renderpass/renderpass.hpp"
#include "../graphics/resource/buffer.hpp"
#include "../graphics/resource/image-storage.hpp"
#include "../graphics/resource/image-user.hpp"
#include "../graphics/resource/mesh.hpp"
#include "../graphics/resource/sampler.hpp"

namespace api {

inline void createBuffer(OrgeCreateBufferParam *param) {
	graphics::resource::addBuffer(
		param->id,
		param->size,
		static_cast<bool>(param->isStorage),
		static_cast<bool>(param->isHostCoherent)
	);
}

inline void destroyBuffer(OrgeDestroyBufferParam *param) {
	graphics::resource::destroyBuffer(param->id);
}

inline void updateBuffer(OrgeUpdateBufferParam *param) {
	graphics::resource::getBuffer(param->id).update(param->data);
}

inline void copyBufferTo(OrgeCopyBufferToParam *param) {
	graphics::resource::getBuffer(param->id).copyTo(param->data);
}

inline void updateBufferDescriptor(OrgeUpdateBufferDescriptorParam *param) {
	graphics::renderpass::getRenderPass(param->renderPassId)
		.getPipeline(param->pipelineId)
		.updateBufferDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void updateComputeBufferDescriptor(OrgeUpdateComputeBufferDescriptorParam *param) {
	graphics::compute::getComputePipeline(param->pipelineId)
		.updateBufferDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void loadImage(OrgeLoadImageParam *param) {
	graphics::resource::addUserImageFromFile(param->file);
}

inline void destroyImage(OrgeDestroyImageParam *param) {
	graphics::resource::destroyUserImage(param->file);
}

inline void createStorageImage(OrgeCreateStorageImageParam *param) {
	graphics::resource::addStorageImage(param->id, param->width, param->height, param->format);
}

inline void destroyStorageImage(OrgeDestroyStorageImageParam *param) {
	graphics::resource::destroyStorageImage(param->id);
}

inline void updateImageDescriptor(OrgeUpdateImageDescriptorParam *param) {
	graphics::renderpass::getRenderPass(param->renderPassId)
		.getPipeline(param->pipelineId)
		.updateUserImageDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void updateComputeImageDescriptor(OrgeUpdateComputeImageDescriptorParam *param) {
	graphics::compute::getComputePipeline(param->pipelineId)
		.updateImageDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void updateComputeStorageImageDescriptor(OrgeUpdateComputeStorageImageDescriptorParam *param) {
	graphics::compute::getComputePipeline(param->pipelineId)
		.updateStorageImageDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void createSampler(OrgeCreateSamplerParam *param) {
	graphics::resource::addSampler(
		param->id,
		static_cast<bool>(param->linearMagFilter),
		static_cast<bool>(param->linearMinFilter),
		static_cast<bool>(param->repeat)
	);
}

inline void destroySampler(OrgeDestroySamplerParam *param) {
	graphics::resource::destroySampler(param->id);
}

inline void updateSamplerDescriptor(OrgeUpdateSamplerDescriptorParam *param) {
	graphics::renderpass::getRenderPass(param->renderPassId)
		.getPipeline(param->pipelineId)
		.updateSamplerDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void updateComputeSamplerDescriptor(OrgeUpdateComputeSamplerDescriptorParam *param) {
	graphics::compute::getComputePipeline(param->pipelineId)
		.updateSamplerDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset
		);
}

inline void updateInputAttachmentDescriptor(OrgeUpdateInputAttachmentDescriptorParam *param) {
	graphics::renderpass::getRenderPass(param->renderPassId)
		.getPipeline(param->pipelineId)
		.updateInputAttachmentDescriptor(
			param->id,
			param->set,
			param->index,
			param->binding,
			param->offset,
			graphics::renderer::renderer().getContext().currentIndex()
		);
}

inline void loadMesh(OrgeLoadMeshParam *param) {
	graphics::resource::addMesh(param->id);
}

inline void destroyMesh(OrgeDestroyMeshParam *param) {
	graphics::resource::destroyMesh(param->id);
}

} // namespace api
