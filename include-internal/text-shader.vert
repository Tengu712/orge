#version 450

layout(constant_id = 0) const int CHAR_COUNT = 1;

layout(set = 0, binding = 0) uniform Instance {
	mat4 transform;
	vec4 uv;
	int texId;
} instances[CHAR_COUNT];

layout(location = 0) out vec2 bridgeUV;
layout(location = 1) out flat int bridgeTexId;

void main() {
	vec2 pos[4] = vec2[](
		vec2(-1.0,  1.0),
		vec2(-1.0, -1.0),
		vec2( 1.0, -1.0),
		vec2( 1.0,  1.0)
	);
	vec2 uvs[4] = vec2[](
		vec2(0.0, 1.0),
		vec2(0.0, 0.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0)
	);

	gl_Position = instances[gl_InstanceIndex].transform * vec4(pos[gl_VertexIndex], 0.0, 1.0);
	bridgeUV = vec2(
		instances[gl_InstanceIndex].uv.x + instances[gl_InstanceIndex].uv.z * uvs[gl_VertexIndex].x,
		instances[gl_InstanceIndex].uv.y + instances[gl_InstanceIndex].uv.w * uvs[gl_VertexIndex].y
	);
	bridgeTexId = instances[gl_InstanceIndex].texId;
}
