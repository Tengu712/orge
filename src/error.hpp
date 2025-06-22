//! orgeが外部へ通知するエラーに関するモジュール
//!
//! 最終的にintにstatic_castされて外部へ通知される。
//! 外部は必要であればその値をもとにエラーメッセージを取得することができる。
//!
//! わざわざこのような古臭いAPI設計にしているのは、
//! - orgeが勝手に標準出力・標準エラー出力に干渉することを防ぐため、
//! - また、より多くの言語からorgeを利用できるようにするためである。

#pragma once

enum class Error {
	None = 0,
	CreateInstance,
	CreateWindow,
};

const char *convertErrorMessage(Error from);
