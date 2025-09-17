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

const std::vector<uint32_t> PATTERN_PL_SET_INDICES{0};
const std::vector<uint32_t> INTEGRATION_PL_SET_INDICES{0};

int main() {
	TRY(orgeInitialize());

	TRY(orgeLoadMesh("triangle"));
	TRY(orgeLoadMesh("square"));

	TRY(orgeCreateBuffer("transform", static_cast<uint64_t>(sizeof(float) * 16), 0, 0));
	TRY(orgeCreateSampler("sampler", 1, 1, 0));

	std::array<float, 16> transform{
		0.3f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.4f, 0.0f, 0.0f,
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

		CHECK(orgeBeginRender());
		CHECK(orgeBeginRenderPass("RP"));

		CHECK(orgeUpdateBufferDescriptor(         "RP", "pattern-pl",     "transform",  0, 0, 0, 0));
		CHECK(orgeUpdateInputAttachmentDescriptor("RP", "integration-pl", "mesh-rt",    0, 0, 0, 0));
		CHECK(orgeUpdateInputAttachmentDescriptor("RP", "integration-pl", "pattern-rt", 0, 0, 1, 0));
		CHECK(orgeUpdateSamplerDescriptor(        "RP", "integration-pl", "sampler",    0, 0, 2, 0));

		CHECK(orgeBindMesh("triangle"));
		CHECK(orgeBindPipeline("mesh-pl", nullptr));
		CHECK(orgeDraw(1, 0));

		CHECK(orgeNextSubpass());

		CHECK(orgeBindMesh("square"));
		CHECK(orgeBindPipeline("pattern-pl", PATTERN_PL_SET_INDICES.data()));
		CHECK(orgeDraw(1, 0));

		CHECK(orgeNextSubpass());

		CHECK(orgeBindPipeline("integration-pl", INTEGRATION_PL_SET_INDICES.data()));
		CHECK(orgeDraw(1, 0));

		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
