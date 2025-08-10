#include "graphics.hpp"

#include "../config/config.hpp"

#include <SDL3/SDL_vulkan.h>
#include <utf8cpp/utf8.h>

namespace graphics {

struct TextRenderingInstance {
	alignas(16) float transform[16];
	alignas(16) float uv[4];
	alignas(16) uint32_t texId[4];
};

std::vector<const char *> getInstanceExtensions() {
	Uint32 count = 0;
	const auto extensions_ = SDL_Vulkan_GetInstanceExtensions(&count);
	auto extensions = std::vector(extensions_, extensions_ + static_cast<size_t>(count));
#ifdef __APPLE__
	extensions.push_back("VK_KHR_portability_enumeration");
#endif
	return extensions;
}

std::vector<const char *> getInstanceLayers() {
#ifdef ENABLE_VVL
	return {"VK_LAYER_KHRONOS_validation"};
#else
	return {};
#endif
}

vk::Instance createInstance() {
#ifdef __APPLE__
	const auto flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#else
	const auto flags = vk::InstanceCreateFlags();
#endif
	const auto extensions = getInstanceExtensions();
	const auto layers = getInstanceLayers();
	const auto ai = vk::ApplicationInfo()
		.setPEngineName("orge")
		.setApiVersion(VK_API_VERSION_1_1);
	const auto ci = vk::InstanceCreateInfo()
		.setFlags(flags)
		.setPApplicationInfo(&ai)
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers);
	return vk::createInstance(ci);
}

vk::PhysicalDevice selectPhysicalDevice(const vk::Instance &instance) {
	const auto devices = instance.enumeratePhysicalDevices();
	if (devices.empty()) {
		throw "no physical device found.";
	}
	// TODO: 適切なものを選ぶ
	return devices.front();
}

uint32_t getQueueFamilyIndex(const vk::PhysicalDevice &physicalDevice) {
	const auto props = physicalDevice.getQueueFamilyProperties();
	const auto iter = std::find_if(
		props.cbegin(),
		props.cend(),
		[](const auto &n) { return n.queueFlags & vk::QueueFlagBits::eGraphics; }
	);
	if (iter == props.cend()) {
		throw "failed to get a queue family index.";
	}
	return static_cast<uint32_t>(std::distance(props.cbegin(), iter));
}

vk::Device createDevice(const vk::PhysicalDevice &physicalDevice, uint32_t queueFamilyIndex) {
#ifdef __APPLE__
	const std::array<const char *, 2> extensions{"VK_KHR_swapchain", "VK_KHR_portability_subset"};
#else
	const std::array<const char *, 1> extensions{"VK_KHR_swapchain"};
#endif
	const auto priority = 1.0f;
	const auto qci = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(queueFamilyIndex)
		.setQueuePriorities(priority);
	const auto ci = vk::DeviceCreateInfo()
		.setQueueCreateInfos(qci)
		.setPEnabledExtensionNames(extensions);
	return physicalDevice.createDevice(ci);
}

Graphics::Graphics() :
	_instance(createInstance()),
	_physicalDevice(selectPhysicalDevice(_instance)),
	_queueFamilyIndex(getQueueFamilyIndex(_physicalDevice)),
	_device(createDevice(_physicalDevice, _queueFamilyIndex)),
	_queue(_device.getQueue(_queueFamilyIndex, 0)),
	_commandPool(_device.createCommandPool(
		vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(_queueFamilyIndex)
	)),
	_renderer(_instance, _physicalDevice, _device, _commandPool)
{
	initializeUtils(_device, _commandPool);

	// すべてのフォントの文字テクスチャアトラスを作成
	for (const auto &n: config::config().fonts) {
		_charAtluss.emplace(n.id, CharAtlus(_physicalDevice.getMemoryProperties(), _device, _queue, n));
	}

	// すべてのテキストレンダリングパイプラインのインスタンスバッファを作成
	for (const auto &n: config::config().pipelines) {
		if (!n.textRendering) {
			continue;
		}
		_buffers.emplace(
			"@buffer@" + n.id,
			Buffer(_physicalDevice.getMemoryProperties(), _device, sizeof(TextRenderingInstance) * n.charCount, true)
		);
	}

	// テキストレンダリングパイプライン用のサンプラ作成
	_samplers.emplace("@sampler@", _device.createSampler(
		vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setMaxLod(vk::LodClampNone)
	));
}

void Graphics::putText(
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
	const auto &extent = _renderer.getExtent();
	const auto extentW = static_cast<float>(extent.width);
	const auto extentH = static_cast<float>(extent.height);

	// すべてのメッシュを左上原点にする
	x += meshSize / 2.0f;
	y += meshSize / 2.0f;

	// Y座標をベースラインに移す
	y += charAtlus.calcAscent(height);

	// すべての文字に対してとりあえず構築
	auto itr = text.begin();
	auto end = text.end();
	std::vector<TextRenderingInstance> instances;
	float minX = x;
	float maxX = x;
	uint32_t lfCount = 0;
	uint32_t count = 0;
	while (itr != end) {
		const auto codepoint = static_cast<uint32_t>(utf8::next(itr, end));

		// TODO: \rは無視、\nは改行

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
		n.transform[12] = x + c->ox;
		n.transform[13] = y + c->oy;
		n.transform[15] = 1.0f;
		n.uv[0] = c->u;
		n.uv[1] = c->v;
		n.uv[2] = ru;
		n.uv[3] = rv;
		n.texId[0] = texId;
		x += c->advance;

		minX = std::min(minX, n.transform[12]);
		maxX = std::max(maxX, n.transform[12] + c->w);

		count += 1;
	}

	// 描画すべき文字がないなら終了
	if (instances.empty()) {
		return;
	}

	// 文字列全体のサイズを計算
	const auto entireWidth  = maxX - minX;
	const auto entireHeight = meshSize + charAtlus.calcLineAdvance(height) * lfCount;

	// 座標修正
	for (auto &n: instances) {
		// X座標を修正
		switch (horizontal) {
		case ORGE_TEXT_LOCATION_HORIZONTAL_LEFT:
			break;
		case ORGE_TEXT_LOCATION_HORIZONTAL_CENTER:
			n.transform[12] -= entireWidth / 2.0f;
			break;
		case ORGE_TEXT_LOCATION_HORIZONTAL_RIGHT:
			n.transform[12] -= entireWidth;
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

		// クリッピング座標系へ
		n.transform[0] /= extentW;
		n.transform[5] /= extentH;
		n.transform[12] -= extentW / 2.0f;
		n.transform[12] /= extentW / 2.0f;
		n.transform[13] -= extentH / 2.0f;
		n.transform[13] /= extentH / 2.0f;
	}

	// アップロード
	error::at(_buffers, "@buffer@" + pipelineId, "buffers")
		.update(
			_device,
			instances.data(),
			instances.size() * sizeof(TextRenderingInstance),
			_textOffset.contains(pipelineId) ? static_cast<size_t>(_textOffset[pipelineId]) : 0
		);

	// offsetを進める
	if (_textOffset.contains(pipelineId)) {
		_textOffset[pipelineId] += count;
	} else {
		_textOffset.emplace(pipelineId, count);
	}
}

void Graphics::beginRender() {
	for (const auto &n: config::config().pipelines) {
		if (!n.textRendering) {
			continue;
		}
		const auto &pipeline = _renderer.getPipeline(n.id);
		pipeline.updateBufferDescriptor(_device, error::at(_buffers, "@buffer@" + n.id, "buffers"), 0, 0, 0, 0);
		for (const auto &[k, v]: _charAtluss) {
			pipeline.updateImageDescriptor(_device, v.get(), 1, 0, 0, error::at(config::config().fontMap, k, "fonts"));
		}
		pipeline.updateSamplerDescriptor(_device, error::at(_samplers, "@sampler@", "samplers"), 1, 0, 1, 0);
	}

	_renderer.beginRender(_device);
}

} // namespace graphics
