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
	std::vector<float> buffer{2, 3, 4};

	TRY(orgeInitialize());

	TRY(orgeCreateBuffer("buffer", static_cast<uint64_t>(sizeof(float) * buffer.size()), 1, 1));
	TRY(orgeUpdateBuffer("buffer", reinterpret_cast<const uint8_t *>(buffer.data())));

	while (orgeUpdate()) {
		const auto shouldDispatch = orgeGetKeyState(static_cast<uint32_t>(ORGE_SCANCODE_RETURN)) == 1;

		CHECK(orgeUpdateComputeBufferDescriptor("PL", "buffer", 0, 0, 0, 0));

		CHECK(orgeBeginRender());
		if (shouldDispatch) {
			CHECK(orgeBindComputePipeline("PL", SETS.data()));
			CHECK(orgeDispatch(1, 1, 1));
		}
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());

		if (shouldDispatch) {
			CHECK(orgeCopyBufferTo("buffer", reinterpret_cast<uint8_t *>(buffer.data())));
			std::cout << buffer[0] << " " << buffer[1] << " " << buffer[2] << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
