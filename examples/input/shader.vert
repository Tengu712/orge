#version 450

void main() {
	vec2 pos[3] = vec2[](
		vec2( 0.0, -1.0),
		vec2(-1.0,  1.0),
		vec2( 1.0,  1.0)
	);
	gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
}
