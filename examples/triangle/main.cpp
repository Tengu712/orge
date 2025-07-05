#include <iostream>
#include <orge.h>
#include <vector>

const std::vector<float> VERTICES{
	// 左下
	-1.0f, -1.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	// 上
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	// 右下
	1.0f, -1.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{
	0, 1, 2,
};

int main() {
	if (!orgeInitializeWith("config.yml")) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}
	if (!orgeCreateMesh("triangle", VERTICES.size(), VERTICES.data(), INDICES.size(), INDICES.data())) {
		std::cout << orgeGetErrorMessage() << std::endl;
		return 1;
	}

	while (orgePollEvents()) {
		const auto result =
			orgeBeginRender()
			&& orgeDraw(0, nullptr)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
