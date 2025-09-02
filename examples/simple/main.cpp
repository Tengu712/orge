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

	int count = 0;
	while (orgeUpdate()) {
		CHECK(orgeBeginRender());
		CHECK(orgeBeginRenderPass("RP"));
		CHECK(orgeEndRenderPass());
		CHECK(orgeEndRender());

		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	orgeTerminate();
	return 0;
}
