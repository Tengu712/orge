#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

#define TRY_CONTINUE(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		continue; \
	}

const std::vector<float> TRIANGLE_VERTICES{
	// 左下
	-1.0f, 1.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	// 上
	0.0f, -1.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	// 右下
	1.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
};
const std::vector<uint32_t> TRIANGLE_INDICES{0, 1, 2};

const std::vector<float> SQUARE_VERTICES{
	// 左下
	-1.0f, 1.0f, 0.0f,
	0.0f, 1.0f,
	// 左上
	-1.0f, -1.0f, 0.0f,
	0.0f, 0.0f,
	// 右上
	1.0f, -1.0f, 0.0f,
	1.0f, 0.0f,
	// 右下
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f,
};
const std::vector<uint32_t> SQUARE_INDICES{0, 1, 2, 0, 2, 3};

const std::vector<uint32_t> PATTERN_PL_SET_INDICES{0};
const std::vector<uint32_t> INTEGRATION_PL_SET_INDICES{0};

int main() {
	TRY(orgeInitializeWith("config.yml"));

	TRY(orgeCreateMesh(
		"triangle",
		static_cast<uint32_t>(TRIANGLE_VERTICES.size()),
		TRIANGLE_VERTICES.data(),
		static_cast<uint32_t>(TRIANGLE_INDICES.size()),
		TRIANGLE_INDICES.data())
	);
	TRY(orgeCreateMesh(
		"square",
		static_cast<uint32_t>(SQUARE_VERTICES.size()),
		SQUARE_VERTICES.data(),
		static_cast<uint32_t>(SQUARE_INDICES.size()),
		SQUARE_INDICES.data())
	);

	TRY(orgeCreateBuffer("time", static_cast<uint64_t>(sizeof(float)), false));
	TRY(orgeUpdateBufferDescriptor("time", "pattern-pl", 0, 0, 0));

	TRY(orgeCreateSampler("sampler", 0, 0, 0));
	TRY(orgeUpdateSamplerDescriptor("sampler", "integration-pl", 0, 0, 2));

	float time = 0.0f;
	while (orgeUpdate()) {
		time += 0.025f;
		TRY(orgeUpdateBuffer("time", &time));

		TRY(orgeBeginRender());
		TRY(orgeDraw("mesh-pl", "triangle", 1, 0));
		orgeNextSubpass();
		TRY(orgeBindDescriptorSets("pattern-pl", PATTERN_PL_SET_INDICES.data()));
		TRY(orgeDraw("pattern-pl", "square", 1, 0));
		orgeNextSubpass();
		TRY(orgeBindDescriptorSets("integration-pl", INTEGRATION_PL_SET_INDICES.data()));
		TRY(orgeDraw("integration-pl", "square", 1, 0));
		TRY(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
