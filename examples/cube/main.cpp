#include <iostream>
#include <orge.h>
#include <vector>

#define TRY(n) \
	if (!(n)) { \
		std::cout << orgeGetErrorMessage() << std::endl; \
		return 1; \
	}

const std::vector<float> VERTICES{
	// 左下
	-1.0f, 1.0f, 0.0f,
	// 左上
	-1.0f, -1.0f, 0.0f,
	// 右上
	1.0f, -1.0f, 0.0f,
	// 右下
	1.0f, 1.0f, 0.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2, 0, 2, 3};
const std::vector<const char *> PIPELINES{"PL"};
const std::vector<float> CAMERA{
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};
const std::vector<uint32_t> SET_INDICES{0};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"cube",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);
	TRY(orgeCreateBuffer("camera", static_cast<uint32_t>(sizeof(float) * CAMERA.size()), 0));
	TRY(orgeUpdateBuffer("camera", CAMERA.data()));
	TRY(orgeUpdateBufferDescriptor("camera", "PL", 0, 0, 0));
	TRY(orgeCreateImageFromFile("image", "image.png"));

	while (orgePollEvents()) {
		const auto result =
			orgeBeginRender()
			&& orgeBindDescriptorSets("PL", SET_INDICES.data())
			&& orgeDraw(static_cast<uint32_t>(PIPELINES.size()), PIPELINES.data(), "cube", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
