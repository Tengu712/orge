#version 450

//struct Instance {
//	mat4 transform;
//	vec4 uv;
//	ivec4 texId;
//};

layout(set = 0, binding = 0) restrict readonly buffer Instances {
	//Instance instances[];
	mat4 instances[];
};

layout(location = 0) out vec2 bridgeUV;
layout(location = 1) out flat int bridgeTexId;

void main() {
	vec2 pos[4] = vec2[](
		vec2(-1.0, -1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0, -1.0),
		vec2( 1.0,  1.0)
	);
	vec2 uvs[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0)
	);

	gl_Position = instances[gl_InstanceIndex] * vec4(pos[gl_VertexIndex], 0.0, 1.0);
	//bridgeUV = vec2(
	//	instances[gl_InstanceIndex].uv.x + instances[gl_InstanceIndex].uv.z * uvs[gl_VertexIndex].x,
	//	instances[gl_InstanceIndex].uv.y + instances[gl_InstanceIndex].uv.w * uvs[gl_VertexIndex].y
	//);
	bridgeUV = uvs[gl_VertexIndex];
	//bridgeTexId = instances[gl_InstanceIndex].texId.r;
	bridgeTexId = 0;
}
