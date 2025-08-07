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

const std::vector<float> VERTICES{
	-1.0f, 1.0f, 0.0f,
	0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f,
	0.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2, 0, 2, 3};

int main() {
	TRY(orgeInitializeWith("config.yml"));
	TRY(orgeCreateMesh(
		"square",
		static_cast<uint32_t>(VERTICES.size()),
		VERTICES.data(),
		static_cast<uint32_t>(INDICES.size()),
		INDICES.data())
	);

	while (orgeUpdate()) {
		CHECK(orgeBeginRender());
		CHECK(orgeDraw("PL", "square", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
