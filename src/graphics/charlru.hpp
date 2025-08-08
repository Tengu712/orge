#pragma once

#include "../error/error.hpp"
#include "character.hpp"

#include <unordered_map>

namespace graphics {

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
	bool has(uint32_t codepoint) {
		return _map.contains(codepoint);
	}

	/// codepointで示される文字を利用する関数
	///
	/// 利用された文字は最新の文字として更新される。
	const Character &use(uint32_t codepoint) {
		auto &node = error::at(_map, codepoint, "rasterized chars");
		// 既に最新のノードなら早期リターン
		if (!node->prev) {
			return node->character;
		}
		// 最古のノードでないなら前後のノードを接続
		if (node->next) {
			node->next->prev = node->prev;
			node->prev->next = node->next;
		}
		// 最古のノードなら先のノードを最古のノードとして登録
		else {
			_oldest = node->prev;
			node->prev->next = nullptr;
		}
		// セットアップ
		node->prev = nullptr;
		node->next = _newest;
		_newest->prev = node;
		_newest = node;
		return node->character;
	}

	/// 飽和状態である場合に限り最古の文字を削除する関数
	///
	/// 削除された場合、trueが返る。
	/// 削除された場合、削除された文字の各情報が引数に格納される。
	/// 飽和状態に達してからはこの情報を描画位置とすること。
	bool popOldestIfSaturated(uint32_t &offsetX, uint32_t &offsetY) noexcept {
		if (static_cast<uint32_t>(_map.size()) < _maxCount) {
			return false;
		}

		const auto deleting = _oldest;
		offsetX = deleting->character.offsetX;
		offsetY = deleting->character.offsetY;

		_oldest->prev->next = nullptr;
		_oldest = _oldest->prev;

		_map.erase(deleting->codepoint);
		delete deleting;
		return true;
	}

	/// 文字を登録する関数
	///
	/// codepointは固有であること。
	/// 飽和状態にないこと。
	void put(uint32_t codepoint, const Character &character) {
		if (_map.contains(codepoint)) {
			throw "duplicated key added to LRU cache.";
		}
		if (static_cast<uint32_t>(_map.size()) >= _maxCount) {
			throw "char atlus is over.";
		}

		auto node = new Node(codepoint, character);
		node->next = _newest;
		if (_newest) {
			_newest->prev = node;
		}
		_newest = node;
		if (!_oldest) {
			_oldest = node;
		}
		_map.emplace(codepoint, node);
	}
};

} // namespace graphics
