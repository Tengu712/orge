#version 450

layout(set = 0, binding = 0) uniform texture2D tex;
layout(set = 0, binding = 1) uniform sampler smplr;

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0) * texture(sampler2D(tex, smplr), bridgeUV).r;
}
