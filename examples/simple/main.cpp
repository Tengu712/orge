#include <iostream>
#include <orge/orge.h>

#define TRY(type, param) if (orgeApiCall((type), (param)) != ORGE_OK) return 1;
#define CHECK(type, param) if (orgeApiCall((type), (param)) != ORGE_OK) continue;

int main() {
	const char *renderTargetId = "RT";
	OrgeAttachmentConfig attachmentConfig{};
	attachmentConfig.id = renderTargetId;
	attachmentConfig.format = ORGE_FORMAT_RENDER_TARGET;
	attachmentConfig.clearValueType = ORGE_CLEAR_VALUE_TYPE_COLOR;
	attachmentConfig.clearValue[0] = 0.894f;
	attachmentConfig.clearValue[1] = 0.619f;
	attachmentConfig.clearValue[2] = 0.38f;
	attachmentConfig.clearValue[3] = 1.0f;
	OrgeSubpassConfig subpassConfig{};
	subpassConfig.id = "SP";
	subpassConfig.outputCount = 1;
	subpassConfig.outputs = &renderTargetId;
	OrgeRenderPassConfig renderPassConfig{};
	renderPassConfig.id = "RP";
	renderPassConfig.subpassCount = 1;
	renderPassConfig.subpasses = &subpassConfig;
	OrgeInitializeParam config{};
	config.title = "simple";
	config.width = 640;
	config.height = 480;
	config.attachmentCount = 1;
	config.attachments = &attachmentConfig;
	config.renderPassCount = 1;
	config.renderPasses = &renderPassConfig;

	TRY(ORGE_INITIALIZE, &config);

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
