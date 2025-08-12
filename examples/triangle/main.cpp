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
	1.0f, 0.0f, 0.0f, 1.0f,
	// 上
	0.0f, -1.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	// 右下
	1.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2};

int main() {
	TRY(orgeInitialize());
	TRY(orgeCreateMesh("triangle"));

	while (orgeUpdate()) {
		const auto result =
			orgeBeginRender()
			&& orgeDraw("PL", "triangle", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
