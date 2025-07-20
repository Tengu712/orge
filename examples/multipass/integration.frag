#version 450

layout(set = 0, binding = 0) uniform texture2D siMesh;
layout(set = 0, binding = 1, input_attachment_index = 1) uniform subpassInput siPattern;
layout(set = 0, binding = 2) uniform sampler smplr;

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
	vec2 distort = subpassLoad(siPattern).rg - 0.5;
	vec2 distortedUV = bridgeUV + distort;
	outColor = texture(sampler2D(siMesh, smplr), distortedUV);
}
