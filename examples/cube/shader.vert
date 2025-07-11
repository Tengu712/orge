#version 450

layout(set = 0, binding = 0) uniform Camera {
    mat4 camera;
} camera;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 bridgeColor;

void main() {
    gl_Position = camera.camera * vec4(inPos, 1.0);
}
