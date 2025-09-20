#include "api/dispatch.hpp"
#include "error/vulkan.hpp"

OrgeApiResult orgeApiCall(OrgeApiCallType type, void *param) {
	try {
		return api::dispatchApi(type, param);
	} catch (const char *e) { \
		error::setMessage(std::string(e)); \
	} catch (const std::string &e) { \
		error::setMessage(e); \
	} catch (const vk::Result &e) { \
		error::handleVkResult(e); \
	} catch (const vk::SystemError &e) { \
		error::handleVkResult(static_cast<vk::Result>(e.code().value())); \
	} catch (const std::exception &e) { \
		error::setMessage(e.what()); \
	} catch (...) { \
		error::setMessage("unbound error."); \
	}
	return ORGE_ERROR;
}
