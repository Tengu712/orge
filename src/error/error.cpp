#include "error.hpp"

namespace error {

std::string g_message;

void setMessage(const std::string &e) {
	g_message = e;
}

} // namespace error

const char *orgeGetErrorMessage(void) {
	return error::g_message.c_str();
}
