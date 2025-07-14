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
const std::vector<const char *> PIPELINES{"PL"};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"triangle",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);

	while (orgePollEvents()) {
		const auto result =
			orgeBeginRender()
			&& orgeBindPipelines(static_cast<uint32_t>(PIPELINES.size()), PIPELINES.data())
			&& orgeDraw("triangle", 1, 0)
			&& orgeEndRender();
		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
