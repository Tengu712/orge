#version 450

layout(set = 1, binding = 0) uniform SamplerIndex { uint si; } si;
layout(set = 1, binding = 1) uniform texture2D tex;
layout(set = 1, binding = 2) uniform sampler smplr[2];

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(sampler2D(tex, smplr[si.si]), bridgeUV);
}
