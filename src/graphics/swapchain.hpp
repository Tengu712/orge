#pragma once

#include "../error/error.hpp"

#include <memory>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

namespace graphics {

using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

class Swapchain {
private:
	const Window _window;
	vk::SurfaceKHR _surface;
	vk::Extent2D _extent;
	vk::SwapchainKHR _swapchain;
	std::vector<vk::Image> _images;

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

	void recreateSwapchain(const vk::PhysicalDevice &physicalDevice, const vk::Device &device);

	void recreateSurface(const vk::Instance &instance, const vk::PhysicalDevice &physicalDevice, const vk::Device &device);

	/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
	///
	/// イメージの取得が完了したら与えられたセマフォをシグナルする。
	///
	/// WARN: 対処すべき例外が発生したらSpecialErrorを投げるので、適宜対処すること。
	uint32_t acquireNextImageIndex(const vk::Device &device, const vk::Semaphore &semaphore) const {
		const auto result = device.acquireNextImageKHR(_swapchain, UINT64_MAX, semaphore, nullptr);
		if (result.result == vk::Result::eSuccess) {
			return result.value;
		}
		switch (result.result) {
		case vk::Result::eSuboptimalKHR:
		case vk::Result::eErrorOutOfDateKHR:
			throw error::SpecialError::NeedRecreateSwapchain;
		case vk::Result::eErrorSurfaceLostKHR:
			throw error::SpecialError::NeedRecreateSurface;
		default:
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

	// TODO: エラーハンドリング
	void toggleFullscreen() const noexcept {
		const auto isFullscreen = static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
		SDL_SetWindowFullscreen(_window.get(), !isFullscreen);
		SDL_SyncWindow(_window.get());
	}
};

} // namespace graphics
