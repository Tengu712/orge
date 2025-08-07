#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 bridgeUV;

void main() {
    vec4 pos = vec4(inPos, 1.0);
    gl_Position = pos;
    bridgeUV = inUV;
}
