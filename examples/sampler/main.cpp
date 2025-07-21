#include <array>
#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
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

const std::vector<unsigned char> PIXELS{
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 255, 0, 255,
	255, 0, 0, 255,
};

const std::vector<uint32_t> SET_INDICES{0, 0};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"square",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);

	TRY(orgeCreateBuffer("transform", static_cast<uint64_t>(sizeof(float) * 16), 0));
	TRY(orgeUpdateBufferDescriptor("transform", "PL", 0, 0, 0, 0));

	TRY(orgeCreateBuffer("sampler-index", static_cast<uint64_t>(sizeof(uint32_t)), 0));
	TRY(orgeUpdateBufferDescriptor("sampler-index", "PL", 1, 0, 0, 0));

	TRY(orgeCreateImage("texture", 2, 2, PIXELS.data()));
	TRY(orgeUpdateImageDescriptor("texture", "PL", 1, 0, 1, 0));

	TRY(orgeCreateSampler("nearest", 0, 0, 0));
	TRY(orgeCreateSampler("linear",  1, 1, 0));
	TRY(orgeUpdateSamplerDescriptor("nearest", "PL", 1, 0, 2, 0));
	TRY(orgeUpdateSamplerDescriptor("linear",  "PL", 1, 0, 2, 1));

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

		const auto result =
			orgeUpdateBuffer("transform", transform.data())
			&& orgeUpdateBuffer("sampler-index", &samplerIndex)
			&& orgeBeginRender()
			&& orgeBindDescriptorSets("PL", SET_INDICES.data())
			&& orgeDraw("PL", "square", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
