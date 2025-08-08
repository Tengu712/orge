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
const std::vector<uint32_t> SET_INDICES{0};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"square",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);
	TRY(orgeCreateSampler("sampler", 1, 1, 0));
	TRY(orgeActivateFont("font"));
	TRY(orgePutString("font", "歓迎しますorge"));

	while (orgeUpdate()) {
		CHECK(orgeUpdateImageDescriptor("font", "PL", 0, 0, 0, 0));
		CHECK(orgeUpdateSamplerDescriptor("sampler", "PL", 0, 0, 1, 0));

		CHECK(orgeBeginRender());
		CHECK(orgeBindDescriptorSets("PL", SET_INDICES.data()));
		CHECK(orgeDraw("PL", "square", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
