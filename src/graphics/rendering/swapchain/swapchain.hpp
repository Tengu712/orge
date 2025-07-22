#pragma once

#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

namespace graphics::rendering::swapchain {

using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

class Swapchain {
private:
	const Window _window;
	const vk::SurfaceKHR _surface;
	const vk::Extent2D _extent;
	const vk::SwapchainKHR _swapchain;
	const std::vector<vk::Image> _images;

public:
	Swapchain(const Swapchain &)  = delete;
	Swapchain(const Swapchain &&) = delete;
	Swapchain &operator=(const Swapchain &)  = delete;
	Swapchain &operator=(const Swapchain &&) = delete;

	Swapchain() = delete;
	Swapchain(
		const std::string &title,
		uint32_t width,
		uint32_t height,
		bool fullscreen,
		const vk::Instance &instance,
		const vk::PhysicalDevice &physicalDevice,
		const vk::Device &device
	);

	void destroy(const vk::Instance &instance, const vk::Device &device) const noexcept {
		device.destroySwapchainKHR(_swapchain);
		instance.destroySurfaceKHR(_surface);
	}

	const vk::Extent2D &getExtent() const noexcept {
		return _extent;
	}

	const std::vector<vk::Image> &getImages() const noexcept {
		return _images;
	}

	bool isFullscreen() const noexcept {
		return static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
	}

	/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
	///
	/// イメージの取得が完了したら与えられたセマフォをシグナルする。
	uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore) const {
		// TODO: ここでスワップチェインの有効性を確認できるので、
		//       独自の例外をキャッチしてもらってSwapchainを再作成してもらう。
		return device.acquireNextImageKHR(_swapchain, UINT64_MAX, semaphore, nullptr).value;
	}

	/// プレゼンテーションを行う関数
	///
	/// 与えられたセマフォがシグナルされるまでプレゼンテーションを待機する。
	/// プレゼンテーションが完了するまでスレッドを待機する。
	void present(const vk::Queue &queue, const vk::Semaphore &semaphore, uint32_t index) const {
		const auto pi = vk::PresentInfoKHR()
			.setWaitSemaphores({semaphore})
			.setSwapchains({_swapchain})
			.setImageIndices({index});
		if (queue.presentKHR(pi) != vk::Result::eSuccess) {
			throw "failed to present the screen.";
		}
	}
};

} // namespace graphics::rendering::swapchain
