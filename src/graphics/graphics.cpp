#include "graphics.hpp"

#include "renderer.hpp"
#include "window.hpp"

#define CHECK(n) if (const auto e = (n); e != Error::None) return e;

namespace graphics {

Error initialize(const char *title, int width, int height) {
	CHECK(renderer::createInstance());
	CHECK(window::createWindow(title, width, height));
	return Error::None;
}

void terminate() {
	window::terminate();
	renderer::terminate();
}

} // namespace graphics
