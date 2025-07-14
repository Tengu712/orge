#include <array>
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
	0.0f, 1.0f,
	// 左上
	-1.0f, -1.0f, 0.0f,
	0.0f, 0.0f,
	// 右上
	1.0f, -1.0f, 0.0f,
	1.0f, 0.0f,
	// 右下
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{0, 1, 2, 0, 2, 3};
const std::vector<const char *> PIPELINES{"PL"};
const std::array<float, 16> CAMERA{
	2.0f/640.0f, 0.0f,        0.0f,         0.0f,
	0.0f,        2.0f/480.0f, 0.0f,         0.0f,
	0.0f,        0.0f,        2.0f/1000.0f, 0.0f,
	0.0f,        0.0f,        0.0f,         1.0f,
};
const std::array<float, 16> SCL{
	100.0f,   0.0f, 0.0f, 0.0f,
	  0.0f, 100.0f, 0.0f, 0.0f,
	  0.0f,   0.0f, 1.0f, 0.0f,
	  0.0f,   0.0f, 0.0f, 1.0f,
};
const std::array<float, 16> TRS{
	1.0f, 0.0f,   0.0f, 0.0f,
	0.0f, 1.0f,   0.0f, 0.0f,
	0.0f, 0.0f,   1.0f, 0.0f,
	0.0f, 0.0f, 250.0f, 1.0f,
};
const std::vector<uint32_t> SET_INDICES{0, 0, 0};

std::array<float, 16> rotY(float ang) {
	float c = std::cos(ang);
	float s = std::sin(ang);
	return {
		   c, 0.0f,    s, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		  -s, 0.0f,    c, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
}

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

	TRY(orgeCreateBuffer("scl", static_cast<uint32_t>(sizeof(float) * SCL.size()), 0));
	TRY(orgeCreateBuffer("rot", static_cast<uint32_t>(sizeof(float) *         16), 0));
	TRY(orgeCreateBuffer("trs", static_cast<uint32_t>(sizeof(float) * TRS.size()), 0));
	TRY(orgeUpdateBuffer("scl", SCL.data()));
	TRY(orgeUpdateBuffer("trs", TRS.data()));
	TRY(orgeUpdateBufferDescriptor("scl", "PL", 1, 0, 0));
	TRY(orgeUpdateBufferDescriptor("rot", "PL", 1, 0, 1));
	TRY(orgeUpdateBufferDescriptor("trs", "PL", 1, 0, 2));

	TRY(orgeCreateImageFromFile("image", "image.png", 0, 0, 0));
	TRY(orgeUpdateImageDescriptor("image", "PL", 2, 0, 0));

	float ang = 0.0f;
	while (orgePollEvents()) {
		ang += 0.01f;
		const auto rot = rotY(ang);

		const auto result =
			orgeUpdateBuffer("rot", rot.data())
			&& orgeBeginRender()
			&& orgeBindPipelines(static_cast<uint32_t>(PIPELINES.size()), PIPELINES.data())
			&& orgeBindDescriptorSets("PL", SET_INDICES.data())
			&& orgeDraw("cube", 1, 0)
			&& orgeEndRender();

		if (!result) {
			std::cout << orgeGetErrorMessage() << std::endl;
		}
	}

	orgeTerminate();
	return 0;
}
