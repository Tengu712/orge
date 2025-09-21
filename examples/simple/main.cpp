#include <iostream>
#include <orge/orge.h>

#define TRY(type, param) if (orgeApiCall((type), (param)) != ORGE_OK) return 1;
#define CHECK(type, param) if (orgeApiCall((type), (param)) != ORGE_OK) continue;

int main() {
	TRY(ORGE_INITIALIZE, nullptr);

	OrgeBeginRenderPassParam beginRenderPassParam{"RP"};

	int count = 0;
	while (orgeApiCall(ORGE_UPDATE, nullptr) != ORGE_WINDOW_CLOSED) {
		CHECK(ORGE_BEGIN_RENDER, nullptr);
		CHECK(ORGE_BEGIN_RENDER_PASS, &beginRenderPassParam);
		CHECK(ORGE_END_RENDER_PASS, nullptr);
		CHECK(ORGE_END_RENDER, nullptr);
		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	TRY(ORGE_TERMINATE, nullptr);
	return 0;
}
