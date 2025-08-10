#version 450

layout(constant_id = 0) const int TEX_COUNT = 1;

layout(set = 1, binding = 0) uniform texture2D tex[TEX_COUNT];
layout(set = 1, binding = 1) uniform sampler smplr;

layout(location = 0) in vec2 bridgeUV;
layout(location = 1) in flat int bridgeTexId;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(1.0) * texture(sampler2D(tex[bridgeTexId], smplr), bridgeUV).r;
}
