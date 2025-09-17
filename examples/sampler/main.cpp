#include <array>
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

const std::vector<uint32_t> SET_INDICES{0, 0};

int main() {
	TRY(orgeInitialize());

	TRY(orgeCreateBuffer("transform", static_cast<uint64_t>(sizeof(float) * 16), 0, 0));
	TRY(orgeCreateBuffer("sampler-index", static_cast<uint64_t>(sizeof(uint32_t)), 0, 0));
	TRY(orgeLoadImage("image.png"));
	TRY(orgeCreateSampler("nearest", 0, 0, 0));
	TRY(orgeCreateSampler("linear",  1, 1, 0));

	std::array<float, 16> transform{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	uint32_t samplerIndex = 0;

	TRY(orgeUpdateBuffer("sampler-index", reinterpret_cast<const uint8_t *>(&samplerIndex)));
	TRY(orgeUpdateBuffer("transform", reinterpret_cast<const uint8_t *>(transform.data())));

	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1) {
			samplerIndex = (samplerIndex + 1) % 2;
			CHECK(orgeUpdateBuffer("sampler-index", reinterpret_cast<const uint8_t *>(&samplerIndex)));
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_I)) > 0) {
			transform[0] += 0.01f;
			transform[5] += 0.01f;
			CHECK(orgeUpdateBuffer("transform", reinterpret_cast<const uint8_t *>(transform.data())));
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_O)) > 0) {
			transform[0] -= 0.01f;
			transform[5] -= 0.01f;
			if (transform[0] < 0.0f) {
				transform[0] = 0.0f;
			}
			if (transform[5] < 0.0f) {
				transform[5] = 0.0f;
			}
			CHECK(orgeUpdateBuffer("transform", reinterpret_cast<const uint8_t *>(transform.data())));
		}

		CHECK(orgeUpdateBufferDescriptor( "RP", "PL", "transform",     0, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor( "RP", "PL", "sampler-index", 1, 0, 0, 0));
		CHECK(orgeUpdateImageDescriptor(  "RP", "PL", "image.png",     1, 0, 1, 0));
		CHECK(orgeUpdateSamplerDescriptor("RP", "PL", "nearest",       1, 0, 2, 0));
		CHECK(orgeUpdateSamplerDescriptor("RP", "PL", "linear",        1, 0, 2, 1));

		CHECK(orgeBeginRender());
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeBindPipeline("PL", SET_INDICES.data()));
		CHECK(orgeDrawDirectly(4, 1, 0));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
