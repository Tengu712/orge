#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput siMesh;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput siPattern;

layout(location = 0) in vec2 bridgeUV;

layout(location = 0) out vec4 outColor;

void main() {
// vec2 distort = texture(distortTexture, fragTexCoord).rg;
// vec2 distortedUV = fragTexCoord + distort;
// outColor = texture(sceneTexture, distortedUV);
	outColor = subpassLoad(siMesh);
}
