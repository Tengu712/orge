#version 450

layout(set = 0, binding = 0) uniform Camera { mat4 camera; } camera;

layout(set = 1, binding = 0) uniform Scl { mat4 scl; } scl;
layout(set = 1, binding = 1) uniform Rot { mat4 rot; } rot;
layout(set = 1, binding = 2) uniform Trs { mat4 trs; } trs;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
	gl_Position = camera.camera * trs.trs * rot.rot * scl.scl * vec4(inPos, 1.0);
	outUV = inUV;
}
