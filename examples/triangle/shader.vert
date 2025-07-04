#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 bridgeColor;

void main() {
    vec4 pos = vec4(inPos, 1.0);
    gl_Position = pos;
    bridgeColor = inColor;
}
