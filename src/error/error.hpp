#pragma once

#include <vulkan/vulkan.hpp>

#define TRY(n) \
	try { \
		n; \
		return 1; \
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
		return 0; \
	} catch (const std::string &e) { \
		error::setMessage(e); \
		return 0; \
	} catch (const vk::SystemError &e) { \
		error::setMessage(std::string(e.what())); \
		return 0; \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
		return 0; \
	} catch (...) { \
		error::setMessage("unbound error."); \
		return 0; \
    }

namespace error {

void setMessage(const std::string &e);

} // namespace error
