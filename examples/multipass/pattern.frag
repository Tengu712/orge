#version 450

layout(location = 0) in vec2 bridgeUV;
layout(location = 1) in vec2 bridgeWH;

layout(location = 0) out vec4 outColor;

void main() {
	const float R = 0.49;
	const float dW = bridgeWH.x * 2.0;
	const float dH = bridgeWH.y * 2.0;

	vec2 centerPos = bridgeUV - 0.5;
	float dis = length(centerPos);

	if (dis < R) {
		vec2 o = -2.0 * centerPos * bridgeWH;
		outColor = vec4(o, 0.0, 0.0);
	} else {
		outColor = vec4(0.0);
	}
}
