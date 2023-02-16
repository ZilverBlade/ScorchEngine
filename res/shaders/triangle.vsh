#version 450

const vec2 vertices[] = {
	{ 0.0, -0.5 },
	{ 0.5, 0.5 },
	{ -0.5, 0.5 },	
};

const vec3 colors[] = {
	{ 1.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
};

layout (location = 0) out vec3 color;

void main() {
	
	color = colors[gl_VertexIndex];
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
	
}