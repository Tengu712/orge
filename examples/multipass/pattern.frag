#version 450

layout(set = 0, binding = 0) uniform Time { float time; } time;

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
	const float R = 0.4;
	const float D = 0.1;

	vec2 pos = bridgeUV - 0.5;
	float dis = length(pos);

	if (dis < R) {
		float curve1 = smoothstep(0.0, 1.0, dis / R);
		float curve2 = smoothstep(0.0, 1.0, curve1);
		float depth = (1.0 - curve2) * D;

		vec2 dir = dis > 0.0 ? pos / dis : vec2(0.0);
		vec2 res = dir * depth;

		outColor = vec4(0.5 + res, 0.0, 0.0);
	} else {
		outColor = vec4(0.5);
	}
}
