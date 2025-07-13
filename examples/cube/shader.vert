#version 450

layout(set = 0, binding = 0) uniform Camera {
	mat4 camera;
} camera;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
	gl_Position = camera.camera * vec4(inPos, 1.0);
	outUV = inUV;
}
