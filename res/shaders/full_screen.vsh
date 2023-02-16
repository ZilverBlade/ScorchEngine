#version 450

const vec2 vertices[6] = {
	{ 1.0, -1.0 },
	{ 1.0, 1.0 },
	{ -1.0, -1.0 },
	{ -1.0, -1.0 },
	{ 1.0, 1.0 },
	{ -1.0, 1.0 }
};

const vec2 texCoords[6] = {
	{ 1.0, 0.0 },
	{ 1.0, 1.0 },
	{ 0.0, 0.0 },
	{ 0.0, 0.0 },
	{ 1.0, 1.0 },
	{ 0.0, 1.0 }
};

layout(location = 0) out vec2 fragUV;

void main() {
	fragUV = texCoords[gl_VertexIndex];
	gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
}