#include <array>
#include <cmath>
#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

#define CHECK(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		continue; \
	}

struct Camera {
	float proj[16];
	float view[16];
};
const Camera CAMERA{
	{
		1.0f,     0.0f,    0.0f, 0.0f,
		0.0f,  0.7071f, 0.7071f, 0.0f,
		0.0f, -0.7071f, 0.7071f, 0.0f,
		0.0f,     0.0f, 212.13f, 1.0f,
	},
	{
		1.0f,    0.0f,   0.0f, 0.0f,
		0.0f, 1.3333f,   0.0f, 0.0f,
		0.0f,    0.0f,  1.04f, 1.0f,
		0.0f,    0.0f, -20.4f, 0.0f,
	},
};
const std::array<float, 16> SCL{
	100.0f,   0.0f,   0.0f, 0.0f,
	  0.0f, 100.0f,   0.0f, 0.0f,
	  0.0f,   0.0f, 100.0f, 0.0f,
	  0.0f,   0.0f,   0.0f, 1.0f,
};
const std::vector<uint32_t> SET_INDICES{0, 0, 0};

std::array<float, 16> rotY(float ang) {
	float c = std::cos(ang);
	float s = std::sin(ang);
	return {
		   c, 0.0f,    s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		  -s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
}

int main() {
	TRY(orgeInitialize());
	TRY(orgeLoadMesh("cube"));

	TRY(orgeCreateBuffer("camera", static_cast<uint32_t>(sizeof(Camera)), 0));
	TRY(orgeUpdateBuffer("camera", reinterpret_cast<const uint8_t *>(&CAMERA)));

	TRY(orgeCreateBuffer("scl", static_cast<uint32_t>(sizeof(float) * SCL.size()), 0));
	TRY(orgeCreateBuffer("rot", static_cast<uint32_t>(sizeof(float) *         16), 0));
	TRY(orgeUpdateBuffer("scl", reinterpret_cast<const uint8_t *>(SCL.data())));

	TRY(orgeLoadImage("image.png"));
	TRY(orgeCreateSampler("sampler", 1, 1, 0));

	float ang = 0.0f;
	while (orgeUpdate()) {
		ang += 0.01f;
		const auto rot = rotY(ang);
		CHECK(orgeUpdateBuffer("rot", reinterpret_cast<const uint8_t *>(rot.data())));

		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "camera", 0, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "scl", 1, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("RP", "PL", "rot", 1, 0, 1, 0));
		CHECK(orgeUpdateImageDescriptor("RP", "PL", "image.png", 2, 0, 0, 0));
		CHECK(orgeUpdateSamplerDescriptor("RP", "PL", "sampler", 2, 0, 1, 0));

		CHECK(orgeBeginRender());
		CHECK(orgeBindMesh("cube"));
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeBindPipeline("PL", SET_INDICES.data()));
		CHECK(orgeDraw(0, 1, 0));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
