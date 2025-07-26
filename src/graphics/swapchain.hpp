#pragma once

#include "../error/error.hpp"

#include <memory>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

namespace graphics {

using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

class Swapchain {
private:
	const uint32_t _width;
	const uint32_t _height;
	const Window _window;
	vk::SurfaceKHR _surface;
	vk::Extent2D _extent;
	vk::Format _format;
	vk::SwapchainKHR _swapchain;
	std::vector<vk::Image> _images;

	void _setFullscreen(bool toFullscreen) const noexcept {
		SDL_SetWindowFullscreen(_window.get(), toFullscreen);
		if (!toFullscreen) {
			SDL_SetWindowSize(_window.get(), static_cast<int>(_width), static_cast<int>(_height));
		}
		SDL_SyncWindow(_window.get());
	}

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

	const vk::Format &getFormat() const noexcept {
		return _format;
	}

	const std::vector<vk::Image> &getImages() const noexcept {
		return _images;
	}

	void recreateSwapchain(const vk::PhysicalDevice &physicalDevice, const vk::Device &device);

	void recreateSurface(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device);

	/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
	///
	/// イメージの取得が完了したら与えられたセマフォをシグナルする。
	uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore) const {
		const auto result = device.acquireNextImageKHR(_swapchain, UINT64_MAX, semaphore, nullptr);
		if (result.result == vk::Result::eSuccess) {
			return result.value;
		} else {
			throw result.result;
		}
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

	bool isFullscreen() const noexcept {
		return static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
	}

	void setFullscreen(bool toFullscreen) const noexcept {
		const auto isFullscreen = static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
		if (toFullscreen != isFullscreen) {
			_setFullscreen(toFullscreen);
		}
	}

	void toggleFullscreen() const noexcept {
		const auto isFullscreen = static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
		_setFullscreen(!isFullscreen);
	}
};

} // namespace graphics
