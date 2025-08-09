#version 450

layout(constant_id = 0) const int CHAR_SIZE = 1;

layout(set = 0, binding = 0) uniform Instance {
	mat4 transform;
	vec4 uv;
} instances[CHAR_SIZE];

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 bridgeUV;

void main() {
	gl_Position = instances[gl_InstanceIndex].transform * vec4(inPos, 1.0);
	bridgeUV = vec2(
		instances[gl_InstanceIndex].uv.x + instances[gl_InstanceIndex].uv.z * inUV.x,
		instances[gl_InstanceIndex].uv.y + instances[gl_InstanceIndex].uv.w * inUV.y
	);
}
