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
	TRY(orgeInitialize());

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

	TRY(orgeCreateBuffer("transform", static_cast<uint64_t>(sizeof(float) * 16), false));
	TRY(orgeCreateSampler("sampler", 0, 0, 0));

	std::array<float, 16> transform{
		0.7f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.7f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	while (orgeUpdate()) {
		const auto l = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_LEFT))  > 0;
		const auto r = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RIGHT)) > 0;
		const auto u = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_UP))    > 0;
		const auto d = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_DOWN))  > 0;
		const auto lr = static_cast<int>(r) - static_cast<int>(l);
		const auto ud = static_cast<int>(d) - static_cast<int>(u);
		const auto k = lr * lr > 0 && ud * ud > 0 ? 0.7071f : 1.0f;
		transform[12] += lr * k * 0.01f;
		transform[13] += ud * k * 0.01f;
		CHECK(orgeUpdateBuffer("transform", reinterpret_cast<const uint8_t *>(transform.data())));

		CHECK(orgeUpdateBufferDescriptor("transform", "pattern-pl", 0, 0, 0, 0));
		CHECK(orgeUpdateSamplerDescriptor("sampler", "integration-pl", 0, 0, 2, 0));

		CHECK(orgeBeginRender());
		CHECK(orgeDraw("mesh-pl", "triangle", 1, 0));
		CHECK(orgeNextSubpass());
		CHECK(orgeBindDescriptorSets("pattern-pl", PATTERN_PL_SET_INDICES.data()));
		CHECK(orgeDraw("pattern-pl", "square", 1, 0));
		CHECK(orgeNextSubpass());
		CHECK(orgeBindDescriptorSets("integration-pl", INTEGRATION_PL_SET_INDICES.data()));
		CHECK(orgeDraw("integration-pl", "square", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
