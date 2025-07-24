#include "error.hpp"

namespace error {

std::string g_message;

void setMessage(const std::string &e) {
	g_message = e;
}

const std::string &getMessage() {
	return g_message;
}

} // namespace error
