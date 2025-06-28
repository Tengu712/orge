#include "error.hpp"

const char *convertErrorMessage(Error from) {
	switch (from) {
		using enum Error;
		case None: return "";
		case InvalidConfig: return "invalid config passed to initialize.";
		case ApplyMoltenVKICD: return "failed to apply lib/MoltenVK_icd.json.";
		case CreateInstance: return "failed to create a Vulkan instance.";
		case CreateWindow: return "failed to create a window.";
		case SelectPhysicalDevice: return "failed to select a physical device.";
		case GetQueueFamilyIndex: return "failed to get a queue family index.";
		case CreateDevice: return "failed to create a Vulkan device.";
		case CreateCommandPool: return "failed to create a command pool.";
		case CreateSurface: return "failed to create a surface.";
		case InvalidColorSpace: return "the surface color space is invalid.";
		case DoubleBufferingUnavailable: return "the surface not support double buffering.";
		case CreateSwapchain: return "failed to create a swapchain.";
		case CreateSwapchainImageView: return "failed to create swapchain image views.";
		case CreateSemaphoresForSwapchain: return "failed to create smephores for swapchain.";
		case CreateRenderPass: return "failed to create a render pass.";
		case CreateFramebuffer: return "failed to create a framebuffer.";
		case CreateDummyFenceForAcquireNextImageIndex: return "failed to create a dummy fence for acquiring next image index.";
		case AcquireNextImageIndex: return "failed to acquire next the swapchain image index.";
		case CreateRenderCommandBuffer: return "failed to create a command buffer for rendering.";
		case ResetRenderCommandBuffer: return "failed to reset a command buffer for rendering.";
		case BeginRenderCommandBuffer: return "failed to begin a command buffer for rendering.";
		case EndRenderCommandBuffer: return "failed to end a command buffer for rendering.";
		case SubmitRenderCommandBuffer: return "failed to submit a command buffer for rendering.";
		case Presentation: return "failed to present.";
		default: return "undefined error happened.";
	}
}
