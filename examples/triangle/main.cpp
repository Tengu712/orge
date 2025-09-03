#include <iostream>
#include <orge.h>

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

int main() {
	TRY(orgeInitialize());
	TRY(orgeLoadMesh("triangle"));

	while (orgeUpdate()) {
		CHECK(orgeBeginRender());
		CHECK(orgeBindMesh("triangle"));
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeBindPipeline("PL", nullptr));
		CHECK(orgeDraw(1, 0));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());
	}

	orgeTerminate();
	return 0;
}
