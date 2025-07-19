#version 450

layout(set = 0, binding = 0) uniform Time { float time; } time;

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
	float noise = sin(bridgeUV.x * 20.0 + time.time * 2.0) * sin(bridgeUV.y * 15.0 + time.time * 1.5) * 0.02;
	outColor = vec4(noise, noise, 0.0, 1.0);
}
