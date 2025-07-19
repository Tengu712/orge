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
	0.0f, -1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2};
const std::vector<float> RED  {1.0f, 0.0f, 0.0f, 1.0f};
const std::vector<float> GREEN{0.0f, 1.0f, 0.0f, 1.0f};
const std::vector<float> BLUE {0.0f, 0.0f, 1.0f, 1.0f};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"triangle",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);

	TRY(orgeCreateBuffer("red",   static_cast<uint64_t>(sizeof(float) * RED.size()),   0));
	TRY(orgeCreateBuffer("green", static_cast<uint64_t>(sizeof(float) * GREEN.size()), 0));
	TRY(orgeCreateBuffer("blue",  static_cast<uint64_t>(sizeof(float) * BLUE.size()),  0));
	TRY(orgeUpdateBuffer("red",   RED.data()));
	TRY(orgeUpdateBuffer("green", GREEN.data()));
	TRY(orgeUpdateBuffer("blue",  BLUE.data()));
	TRY(orgeUpdateBufferDescriptor("red",   "PL", 0, 0, 0));
	TRY(orgeUpdateBufferDescriptor("green", "PL", 0, 1, 0));
	TRY(orgeUpdateBufferDescriptor("blue",  "PL", 0, 2, 0));

	uint32_t state = 0;
	while (orgeUpdate()) {
		const auto keyState = orgeGetKeyState(ORGE_SCANCODE_RETURN);
		if (keyState == 1) {
			std::cout << "return key pressed" << std::endl;
			state = (state + 1) % 3;
		}
		if (keyState > 0) {
			std::cout << "return key down" << std::endl;
		}
		if (keyState == -1) {
			std::cout << "return key released" << std::endl;
		}

		const std::vector<uint32_t> sets{state};
		const auto result =
			orgeBeginRender()
			&& orgeBindPipeline("PL")
			&& orgeBindDescriptorSets("PL", sets.data())
			&& orgeDraw("triangle", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
