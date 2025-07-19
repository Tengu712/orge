#version 450

layout(set = 2, binding = 0) uniform texture2D tex;
layout(set = 2, binding = 1) uniform sampler smplr;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(sampler2D(tex, smplr), inUV);
}
