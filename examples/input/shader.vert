#version 450

layout(location = 0) in vec3 inPos;

void main() {
	vec4 pos = vec4(inPos, 1.0);
	gl_Position = pos;
}
