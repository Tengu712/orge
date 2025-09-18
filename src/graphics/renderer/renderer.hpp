#pragma once

#include "context.hpp"

#include <optional>

namespace graphics::renderer {

class Renderer {
private:
	/// 描画処理コマンド用のコマンドバッファ
	/// orgeは描画完了まで待機するので1個で十分
	const vk::UniqueCommandBuffer _commandBuffer;
	/// スワップチェインイメージ取得の完了を知るためのセマフォ
	/// コマンドバッファ提出を待機させるために使う
	vk::UniqueSemaphore _semaphoreForImageEnabled;
	/// コマンドバッファ実行の完了を知るためのセマフォ
	/// プレゼンテーション開始を待機させるために使う
	std::vector<vk::UniqueSemaphore> _semaphoreForRenderFinisheds;
	/// フレーム完了を監視するフェンス
	/// 次フレーム開始前にGPU処理完了を待機するために使う
	const vk::UniqueFence _frameInFlightFence;
	/// レンダリング中の必要な情報をまとめたもの
	std::optional<RenderContext> _context;

public:
	Renderer();

	RenderContext &getContext() {
		if (_context) {
			return _context.value();
		} else {
			throw "rendering not begun.";
		}
	}

	void begin();
	void end();

	void reset();

	// NOTE: vk::Result::eSuboptimalKHRはセマフォをシグナルするらしいので、
	//       直後のacuireNextImageKHRでヴァリデーションエラーが出ないように
	//       セマフォを再作成するためのメソッド。
	void recreateSemaphoreForImageEnabled();
};

void initializeRenderer();

void destroyRenderer() noexcept;

Renderer &renderer();

} // namespace graphics::renderer
