#include <iostream>
#include <orge/orge.h>

#define CALL(type, param) \
	{ \
		const auto res = orgeApiCall(type, param); \
		if (res == ORGE_ERROR) { \
			orgeApiCall(ORGE_SHOW_ERROR_DIALOG, nullptr); \
			return res; \
		} \
	}

int main() {
	CALL(ORGE_INITIALIZE, nullptr);

	OrgeBeginRenderPassParam beginRenderPassParam{"RP"};

	int count = 0;
	while (orgeApiCall(ORGE_UPDATE, nullptr) != ORGE_WINDOW_CLOSED) {
		CALL(ORGE_BEGIN_RENDER, nullptr);
		CALL(ORGE_BEGIN_RENDER_PASS, &beginRenderPassParam);
		CALL(ORGE_END_RENDER_PASS, nullptr);
		CALL(ORGE_END_RENDER, nullptr);
		if (count % 60 == 0) {
			std::cout << count / 60 << std::endl;
		}
		count += 1;
	}

	CALL(ORGE_TERMINATE, nullptr);
	return 0;
}
