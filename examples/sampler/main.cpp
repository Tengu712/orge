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

const std::vector<float> VERTICES{
	-1.0f, 1.0f, 0.0f,
	0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2, 0, 2, 3};

const std::vector<uint8_t> PIXELS{
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 255, 0, 255,
	255, 0, 0, 255,
};

const std::vector<uint32_t> SET_INDICES{0, 0};

int main() {
	TRY(orgeInitialize());
	TRY(orgeCreateMesh(
		"square",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);

	TRY(orgeCreateBuffer("transform", static_cast<uint64_t>(sizeof(float) * 16), 0));
	TRY(orgeCreateBuffer("sampler-index", static_cast<uint64_t>(sizeof(uint32_t)), 0));
	TRY(orgeCreateImage("texture", "image.png"));
	TRY(orgeCreateSampler("nearest", 0, 0, 0));
	TRY(orgeCreateSampler("linear",  1, 1, 0));

	std::array<float, 16> transform{
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	uint32_t samplerIndex = 0;

	while (orgeUpdate()) {
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1) {
			samplerIndex = (samplerIndex + 1) % 2;
		}
		if (orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_I)) > 0) {
			transform[0] += 0.01f;
			transform[5] += 0.01f;
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
		}

		CHECK(orgeUpdateBuffer("transform", reinterpret_cast<const uint8_t *>(transform.data())));
		CHECK(orgeUpdateBuffer("sampler-index", reinterpret_cast<const uint8_t *>(&samplerIndex)));

		CHECK(orgeUpdateBufferDescriptor("transform", "PL", 0, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("sampler-index", "PL", 1, 0, 0, 0));
		CHECK(orgeUpdateImageDescriptor("texture", "PL", 1, 0, 1, 0));
		CHECK(orgeUpdateSamplerDescriptor("nearest", "PL", 1, 0, 2, 0));
		CHECK(orgeUpdateSamplerDescriptor("linear",  "PL", 1, 0, 2, 1));

		CHECK(orgeBeginRender());
		CHECK(orgeBindDescriptorSets("PL", SET_INDICES.data()));
		CHECK(orgeDraw("PL", "square", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
