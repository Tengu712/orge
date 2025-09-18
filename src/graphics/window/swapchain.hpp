#pragma once

#include <memory>
#include <SDL3/SDL.h>
#include <vulkan/vulkan.hpp>

namespace graphics::window {

using Window = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;

class Swapchain {
private:
	const uint32_t _width;
	const uint32_t _height;
	const Window _window;
	vk::UniqueSurfaceKHR _surface;
	vk::Extent2D _extent;
	vk::Format _format;
	vk::UniqueSwapchainKHR _swapchain;
	std::vector<vk::Image> _images;

public:
	Swapchain(const Swapchain &)  = delete;
	Swapchain(const Swapchain &&) = delete;
	Swapchain &operator=(const Swapchain &)  = delete;
	Swapchain &operator=(const Swapchain &&) = delete;

	Swapchain();

	const vk::Extent2D &getExtent() const noexcept {
		return _extent;
	}

	const vk::Format &getFormat() const noexcept {
		return _format;
	}

	const std::vector<vk::Image> &getImages() const noexcept {
		return _images;
	}

	bool isFullscreen() const noexcept {
		return static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
	}

	void setFullscreen(bool toFullscreen) const noexcept {
		const auto isFullscreen = static_cast<bool>(SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_FULLSCREEN);
		if (toFullscreen != isFullscreen) {
			SDL_SetWindowFullscreen(_window.get(), toFullscreen);
			if (!toFullscreen) {
				SDL_SetWindowSize(_window.get(), static_cast<int>(_width), static_cast<int>(_height));
			}
			SDL_SyncWindow(_window.get());
		}
	}

	void recreateSwapchain();

	void recreateSurface();

	/// 利用可能な次のスワップチェインイメージのインデックスを取得する関数
	///
	/// イメージの取得が完了したら与えられたセマフォをシグナルする。
	uint32_t acquireNextImageIndex(const vk::Semaphore &semaphore);

	/// プレゼンテーションを行う関数
	///
	/// 与えられたセマフォがシグナルされるまでプレゼンテーションを待機する。
	/// プレゼンテーションが完了するまでスレッドを待機する。
	void present(const vk::Semaphore &semaphore, uint32_t index) const;
};

void initializeSwapchain();

void destroySwapchain() noexcept;

Swapchain &swapchain();

} // namespace graphics::window
