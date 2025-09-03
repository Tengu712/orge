#version 450

layout(set = 0, binding = 0) uniform Transform { mat4 transform; } transform;

layout(location = 0) out vec2 bridgeUV;

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

	gl_Position = transform.transform * vec4(pos[gl_VertexIndex], 0.0, 1.0);
	bridgeUV = uvs[gl_VertexIndex];
}
