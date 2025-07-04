//! orgeが外部へ通知するエラーに関するモジュール

#pragma once

#include <string>

namespace error {

void setMessage(const std::string &e);

const char *getMessage();

} // namespace error
