#include "error.hpp"

namespace error {

std::string g_message;

void setMessage(const std::string &e) {
	g_message = e;
}

const char *getMessage() {
	return g_message.c_str();
}

} // namespace error
