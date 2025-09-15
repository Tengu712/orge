#pragma once

#include "charatlus-char.hpp"

#include <unordered_map>

namespace graphics::resource {

class CharLru {
private:
	struct Node {
		const uint32_t codepoint;
		const Character character;
		Node *prev; // 新しいノード
		Node *next; // 古いノード

		Node(uint32_t codepoint, const Character &character):
			codepoint(codepoint),
			character(character),
			prev(nullptr),
			next(nullptr)
		{}
	};

	const uint32_t _maxCount;
	Node *_newest;
	Node *_oldest;
	std::unordered_map<uint32_t, Node *> _map;

public:
	CharLru() = delete;

	CharLru(uint32_t maxCount): _maxCount(maxCount), _newest(nullptr), _oldest(nullptr) {}
	~CharLru() {
		for (auto n: _map) {
			delete n.second;
		}
	}

	/// 現在登録されている文字の数を取得する関数
	///
	/// 飽和状態に達するまではこの結果値をもとに描画位置を算出すること。
	uint32_t size() const noexcept {
		return static_cast<uint32_t>(_map.size());
	}

	/// codepointで示される文字が登録されているか確認する関数
	bool has(uint32_t codepoint) const noexcept {
		return _map.contains(codepoint);
	}

	/// codepointで示される文字を利用する関数
	///
	/// 利用された文字は最新の文字として更新される。
	/// 文字が存在しない場合、例外が発生する。
	const Character &use(uint32_t codepoint);

	/// 飽和状態である場合に限り最古の文字を削除する関数
	///
	/// 削除された場合、削除された文字の各情報が引数に格納される。
	/// 飽和状態に達してからはこの情報を描画位置とすること。
	void popOldestIfSaturated(uint32_t &offsetX, uint32_t &offsetY) noexcept;

	/// 文字を登録する関数
	///
	/// codepointは固有であること。
	/// 飽和状態にないこと。
	void put(uint32_t codepoint, const Character &character);
};

} // namespace graphics::resource
