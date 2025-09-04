#version 450

layout(set = 0, binding = 0) uniform Transform { mat4 transform; } transform;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 bridgeUV;
layout(location = 1) out vec2 bridgeWH;

void main() {
	gl_Position = transform.transform * vec4(inPos, 1.0);
	bridgeUV = inUV;
	bridgeWH = vec2(transform.transform[0][0], transform.transform[1][1]);
}
