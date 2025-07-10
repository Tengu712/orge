#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n)                                           \
	if (!(n)) {                                          \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1;                                        \
	}

const std::vector<float> VERTICES{
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
const std::vector<uint32_t> INDICES{0, 1, 2};
const std::vector<const char *> PIPELINES{"PL"};
const std::vector<float> CAMERA{
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh("cube", VERTICES.size(), VERTICES.data(), INDICES.size(), INDICES.data()));
	TRY(orgeCreateBuffer("camera", sizeof(float) * CAMERA.size(), 0));
	TRY(orgeUpdateBuffer("camera", CAMERA.data()));

	while (orgePollEvents()) {
		const auto result =
			orgeBeginRender()
			&& orgeDraw(PIPELINES.size(), PIPELINES.data(), "cube", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
