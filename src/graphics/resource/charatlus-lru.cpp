#include "charatlus-lru.hpp"

#include "../../error/error.hpp"

namespace graphics::resource {

const Character &CharLru::use(uint32_t codepoint) {
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

void CharLru::popOldestIfSaturated(uint32_t &offsetX, uint32_t &offsetY) noexcept {
	if (static_cast<uint32_t>(_map.size()) < _maxCount) {
		return;
	}

	const auto deleting = _oldest;
	offsetX = deleting->character.x;
	offsetY = deleting->character.y;

	_oldest->prev->next = nullptr;
	_oldest = _oldest->prev;

	_map.erase(deleting->codepoint);
	delete deleting;
}

void CharLru::put(uint32_t codepoint, const Character &character) {
	if (_map.contains(codepoint)) {
		throw std::format("unexpected error: duplicated character added: codepoint={}", codepoint);
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

} // namespace graphics::resource
