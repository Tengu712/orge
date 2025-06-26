//! orge初期化用のyamlをパースするモジュール

#pragma once

#include <optional>
#include <string>

struct Config {
	const std::string title;
	const int width;
	const int height;
};

std::optional<Config> parseConfig(const char *const yaml);

std::optional<Config> parseConfigFromFile(const char *const yamlFilePath);
