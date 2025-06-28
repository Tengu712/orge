//! orgeが外部へ通知するエラーに関するモジュール
//!
//! 最終的にintにstatic_castされて外部へ通知される。
//! 外部は必要であればその値をもとにエラーメッセージを取得することができる。
//!
//! わざわざこのような古臭いAPI設計にしているのは、
//! - orgeが勝手に標準出力・標準エラー出力に干渉することを防ぐため、
//! - また、より多くの言語からorgeを利用できるようにするためである。

#pragma once

/// Error値がError::Noneでなければその値を早期returnするマクロ関数
#define CHECK(n) if (auto e = (n); e != Error::None) return e;

enum class Error {
	None = 0,
	InvalidConfig,
	ApplyMoltenVKICD,
	CreateInstance,
	CreateWindow,
	SelectPhysicalDevice,
	GetQueueFamilyIndex,
	CreateDevice,
	CreateCommandPool,
	CreateSurface,
	InvalidColorSpace,
	DoubleBufferingUnavailable,
	CreateSwapchain,
	CreateSwapchainImageView,
	CreateSemaphoresForSwapchain,
	CreateRenderPass,
};

const char *convertErrorMessage(Error from);
