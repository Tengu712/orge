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

std::array<uint32_t, 1> SETS{0};

int main() {
	std::vector<float> input{2, 3, 4};
	std::vector<float> output{0, 0, 0};

	TRY(orgeInitialize());

	TRY(orgeCreateBuffer("input",  static_cast<uint64_t>(sizeof(float) * input.size()),  1, 1));
	TRY(orgeCreateBuffer("output", static_cast<uint64_t>(sizeof(float) * output.size()), 1, 1));
	TRY(orgeUpdateBuffer("input",  reinterpret_cast<const uint8_t *>(input.data())));
	TRY(orgeUpdateBuffer("output", reinterpret_cast<const uint8_t *>(output.data())));

	while (orgeUpdate()) {
		const auto shouldDispatch = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1;

		CHECK(orgeUpdateComputeBufferDescriptor("PL", "input",  0, 0, 0, 0));
		CHECK(orgeUpdateComputeBufferDescriptor("PL", "output", 0, 0, 1, 0));

		CHECK(orgeBeginRender());
		if (shouldDispatch) {
			CHECK(orgeBindComputePipeline("PL", SETS.data()));
			CHECK(orgeDispatch(1, 1, 1));
		}
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());

		if (shouldDispatch) {
			CHECK(orgeCopyBufferTo("output", reinterpret_cast<uint8_t *>(output.data())));
			std::cout << output[0] << " " << output[1] << " " << output[2] << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
