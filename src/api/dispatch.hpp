#pragma once

#include <orge/orge.h>

#include "audio.hpp"
#include "compute.hpp"
#include "dialog.hpp"
#include "input.hpp"
#include "lifetime.hpp"
#include "rendering.hpp"
#include "resource.hpp"
#include "text.hpp"
#include "window.hpp"

namespace api {

inline OrgeApiResult dispatchApi(OrgeApiCallType type, void *param) {
	switch (type) {
	case ORGE_SHOW_DIALOG:
		showDialog(static_cast<OrgeShowDialogParam *>(param));
		break;
	case ORGE_SHOW_ERROR_DIALOG:
		showErrorDialog();
		break;

	case ORGE_INITIALIZE:
		initialize();
		break;
	case ORGE_TERMINATE:
		terminate();
		break;
	case ORGE_UPDATE:
		return update();

	case ORGE_IS_FULLSCREEN:
		isFullscreen(static_cast<OrgeIsFullscreenParam *>(param));
		break;
	case ORGE_SET_FULLSCREEN:
		setFullscreen(static_cast<OrgeSetFullscreenParam *>(param));
		break;

	case ORGE_CREATE_BUFFER:
		createBuffer(static_cast<OrgeCreateBufferParam *>(param));
		break;
	case ORGE_DESTROY_BUFFER:
		destroyBuffer(static_cast<OrgeDestroyBufferParam *>(param));
		break;
	case ORGE_UPDATE_BUFFER:
		updateBuffer(static_cast<OrgeUpdateBufferParam *>(param));
		break;
	case ORGE_COPY_BUFFER_TO:
		copyBufferTo(static_cast<OrgeCopyBufferToParam *>(param));
		break;
	case ORGE_UPDATE_BUFFER_DESCRIPTOR:
		updateBufferDescriptor(static_cast<OrgeUpdateBufferDescriptorParam *>(param));
		break;
	case ORGE_UPDATE_COMPUTE_BUFFER_DESCRIPTOR:
		updateComputeBufferDescriptor(static_cast<OrgeUpdateComputeBufferDescriptorParam *>(param));
		break;
	case ORGE_LOAD_IMAGE:
		loadImage(static_cast<OrgeLoadImageParam *>(param));
		break;
	case ORGE_DESTROY_IMAGE:
		destroyImage(static_cast<OrgeDestroyImageParam *>(param));
		break;
	case ORGE_CREATE_STORAGE_IMAGE:
		createStorageImage(static_cast<OrgeCreateStorageImageParam *>(param));
		break;
	case ORGE_DESTROY_STORAGE_IMAGE:
		destroyStorageImage(static_cast<OrgeDestroyStorageImageParam *>(param));
		break;
	case ORGE_UPDATE_IMAGE_DESCRIPTOR:
		updateImageDescriptor(static_cast<OrgeUpdateImageDescriptorParam *>(param));
		break;
	case ORGE_UPDATE_COMPUTE_IMAGE_DESCRIPTOR:
		updateComputeImageDescriptor(static_cast<OrgeUpdateComputeImageDescriptorParam *>(param));
		break;
	case ORGE_UPDATE_COMPUTE_STORAGE_IMAGE_DESCRIPTOR:
		updateComputeStorageImageDescriptor(static_cast<OrgeUpdateComputeStorageImageDescriptorParam *>(param));
		break;
	case ORGE_CREATE_SAMPLER:
		createSampler(static_cast<OrgeCreateSamplerParam *>(param));
		break;
	case ORGE_DESTROY_SAMPLER:
		destroySampler(static_cast<OrgeDestroySamplerParam *>(param));
		break;
	case ORGE_UPDATE_SAMPLER_DESCRIPTOR:
		updateSamplerDescriptor(static_cast<OrgeUpdateSamplerDescriptorParam *>(param));
		break;
	case ORGE_UPDATE_COMPUTE_SAMPLER_DESCRIPTOR:
		updateComputeSamplerDescriptor(static_cast<OrgeUpdateComputeSamplerDescriptorParam *>(param));
		break;
	case ORGE_UPDATE_INPUT_ATTACHMENT_DESCRIPTOR:
		updateInputAttachmentDescriptor(static_cast<OrgeUpdateInputAttachmentDescriptorParam *>(param));
		break;
	case ORGE_LOAD_MESH:
		loadMesh(static_cast<OrgeLoadMeshParam *>(param));
		break;
	case ORGE_DESTROY_MESH:
		destroyMesh(static_cast<OrgeDestroyMeshParam *>(param));
		break;

	case ORGE_RASTERIZE_CHARACTERS:
		rasterizeCharacters(static_cast<OrgeRasterizeCharactersParam *>(param));
		break;
	case ORGE_LAYOUT_TEXT:
		layoutText(static_cast<OrgeLayoutTextParam *>(param));
		break;
	case ORGE_DRAW_TEXTS:
		drawTexts();
		break;

	case ORGE_BEGIN_RENDER:
		beginRender();
		break;
	case ORGE_END_RENDER:
		endRender();
		break;
	case ORGE_BIND_MESH:
		bindMesh(static_cast<OrgeBindMeshParam *>(param));
		break;
	case ORGE_BEGIN_RENDER_PASS:
		beginRenderPass(static_cast<OrgeBeginRenderPassParam *>(param));
		break;
	case ORGE_END_RENDER_PASS:
		endRenderPass();
		break;
	case ORGE_NEXT_SUBPASS:
		nextSubpass();
		break;
	case ORGE_BIND_PIPELINE:
		bindPipeline(static_cast<OrgeBindPipelineParam *>(param));
		break;
	case ORGE_DRAW:
		draw(static_cast<OrgeDrawParam *>(param));
		break;
	case ORGE_DRAW_DIRECTLY:
		drawDirectly(static_cast<OrgeDrawDirectlyParam *>(param));
		break;

	case ORGE_BIND_COMPUTE_PIPELINE:
		bindComputePipeline(static_cast<OrgeBindComputePipelineParam *>(param));
		break;
	case ORGE_DISPATCH:
		dispatch(static_cast<OrgeDispatchParam *>(param));
		break;

	case ORGE_GET_KEY_STATE:
		getKeyState(static_cast<OrgeGetKeyStateParam *>(param));
		break;
	case ORGE_GET_CURSOR:
		getCursor(static_cast<OrgeGetCursorParam *>(param));
		break;
	case ORGE_GET_MOUSE_BUTTON_STATE:
		getMouseButtonState(static_cast<OrgeGetMouseButtonStateParam *>(param));
		break;

	case ORGE_GET_AUDIO_CHANNEL_VOLUME:
		return getAudioChannelVolume(static_cast<OrgeGetAudioChannelVolumeParam *>(param));
	case ORGE_SET_AUDIO_CHANNEL_VOLUME:
		setAudioChannelVolume(static_cast<OrgeSetAudioChannelVolumeParam *>(param));
		break;
	case ORGE_LOAD_WAVE:
		loadWave(static_cast<OrgeLoadWaveParam *>(param));
		break;
	case ORGE_DESTROY_WAVE:
		destroyWave(static_cast<OrgeDestroyWaveParam *>(param));
		break;
	case ORGE_PLAY_WAVE:
		playWave(static_cast<OrgePlayWaveParam *>(param));
		break;

	default:
		throw std::format("invalid api type: {}", static_cast<uint32_t>(type));
	}

	return ORGE_OK;
}

} // namespace api
