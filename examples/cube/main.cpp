#include <array>
#include <cmath>
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
	// 前
	-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
	// 後
	 0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
	// 左
	-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
	// 右
	 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
	// 上
	-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
	 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
	// 下
	-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
	-0.5f,  0.5f, -0.5f, 0.0f, 0.0f,
	 0.5f,  0.5f, -0.5f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
};
const std::vector<uint32_t> INDICES{
	0, 1, 2, 0, 2, 3,
	4, 5, 6, 4, 6, 7,
	8, 9, 10, 8, 10, 11,
	12, 13, 14, 12, 14, 15,
	16, 17, 18, 16, 18, 19,
	20, 21, 22, 20, 22, 23,
};
struct Camera {
	float proj[16];
	float view[16];
};
const Camera CAMERA{
	{
		1.0f,     0.0f,    0.0f, 0.0f,
		0.0f,  0.7071f, 0.7071f, 0.0f,
		0.0f, -0.7071f, 0.7071f, 0.0f,
		0.0f,     0.0f, 212.13f, 1.0f,
	},
	{
		1.0f,    0.0f,   0.0f, 0.0f,
		0.0f, 1.3333f,   0.0f, 0.0f,
		0.0f,    0.0f,  1.04f, 1.0f,
		0.0f,    0.0f, -20.4f, 0.0f,
	},
};
const std::array<float, 16> SCL{
	100.0f,   0.0f,   0.0f, 0.0f,
	  0.0f, 100.0f,   0.0f, 0.0f,
	  0.0f,   0.0f, 100.0f, 0.0f,
	  0.0f,   0.0f,   0.0f, 1.0f,
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

	TRY(orgeCreateBuffer("camera", static_cast<uint32_t>(sizeof(Camera)), 0));
	TRY(orgeUpdateBuffer("camera", reinterpret_cast<const uint8_t *>(&CAMERA)));

	TRY(orgeCreateBuffer("scl", static_cast<uint32_t>(sizeof(float) * SCL.size()), 0));
	TRY(orgeCreateBuffer("rot", static_cast<uint32_t>(sizeof(float) *         16), 0));
	TRY(orgeUpdateBuffer("scl", reinterpret_cast<const uint8_t *>(SCL.data())));

	TRY(orgeCreateImageFromFile("image", "image.png"));
	TRY(orgeCreateSampler("sampler", 0, 0, 0));

	float ang = 0.0f;
	while (orgeUpdate()) {
		ang += 0.01f;
		const auto rot = rotY(ang);
		CHECK(orgeUpdateBuffer("rot", reinterpret_cast<const uint8_t *>(rot.data())));

		CHECK(orgeUpdateBufferDescriptor("camera", "PL", 0, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("scl", "PL", 1, 0, 0, 0));
		CHECK(orgeUpdateBufferDescriptor("rot", "PL", 1, 0, 1, 0));
		CHECK(orgeUpdateImageDescriptor("image", "PL", 2, 0, 0, 0));
		CHECK(orgeUpdateSamplerDescriptor("sampler", "PL", 2, 0, 1, 0));

		CHECK(orgeBeginRender());
		CHECK(orgeBindDescriptorSets("PL", SET_INDICES.data()));
		CHECK(orgeDraw("PL", "cube", 1, 0));
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
