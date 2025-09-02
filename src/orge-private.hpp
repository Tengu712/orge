#pragma once

#include "error/error.hpp"

#include <vulkan/vulkan.hpp>

#define CHECK(n) \
	try { \
		n; \
		result = true; \
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
	} catch (const std::string &e) { \
		error::setMessage(e); \
	} catch (const vk::Result &e) { \
		orge::handleVkResult(e); \
	} catch (const vk::SystemError &e) { \
		orge::handleVkResult(static_cast<vk::Result>(e.code().value())); \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
	} catch (...) { \
		error::setMessage("unbound error."); \
	}
#define TRY(n)         bool result = false; CHECK(n); return static_cast<uint8_t>(result);
#define TRY_DISCARD(n) bool result = false; CHECK(n); (void)result;

namespace orge {

void handleVkResult(const vk::Result &e);

} // namespace orge
