#include "head.hpp"

namespace res {
	std::vector<object> objects;
	GLuint rect_vao = 0;

	namespace shaders {
		GLuint rectangle;

		const char* vertex = R"(
#version 330 core
layout (location = 0) in vec3 pos;
void main() {
	gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
}
)""\0";

		const char* fragment = R"(
#version 330 core
out vec4 outcolour;
uniform vec4 colour;
uniform float noise;
uniform float pxsize;
uniform vec2 offset;
uint jhash(uint x) {
	x += x << 10u;
	x ^= x >> 6u;
	x += x << 3u;
	x ^= x >> 11u;
	x += x << 15u;
	return x;
}
float rand(vec2 v) {
	uint hash = jhash((floatBitsToUint(v).x) ^ jhash(floatBitsToUint(v).y));
	hash = 0x3f800000u | (hash & 0x007fffffu);
	return (uintBitsToFloat(hash) - 1.0) * 6.28318530718;
}
void main() {
	vec2 d = vec2(0.0f);
	outcolour = vec4(colour);
	if (noise != 0.0f) {
		float angle = rand(floor(gl_FragCoord.xy / vec2(pxsize) + offset));
		d = vec2(noise * cos(angle), noise * sin(angle));
	if (max(colour.r, max(colour.g, colour.b)) == colour.r)
		outcolour = vec4(colour.r, colour.g + d.x / 255.0, colour.b + d.y / 255.0, colour.a);
	if (max(colour.r, max(colour.g, colour.b)) == colour.g)
		outcolour = vec4(colour.r + d.y / 255.0, colour.g, colour.b + d.x / 255.0, colour.a);
	if (max(colour.r, max(colour.g, colour.b)) == colour.b)
		outcolour = vec4(colour.r + d.x / 255.0, colour.g + d.y / 255.0, colour.b, colour.a);
	}
}
)""\0";
	}
}

void res::load_vao(void) {
	const GLfloat quad[] = {
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f
	};
	GLuint vbo = 0;
	glGenVertexArrays(1, &rect_vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glBindVertexArray(rect_vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void res::shaders::load(void) {
	rectangle = glCreateProgram();
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertex, NULL);
	glCompileShader(vert);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragment, NULL);
	glCompileShader(frag);
#ifdef _DEBUG
	char log[1024];
	glGetShaderInfoLog(frag, 1024, NULL, log);
	printf("%s", log);
#endif
	glAttachShader(rectangle, vert);
	glAttachShader(rectangle, frag);
	glLinkProgram(rectangle);
	glDeleteShader(vert);
	glDeleteShader(frag);
}

void res::loader::load_tex(void) {
	textures[0].push_back({ 0, 0, 1, 1, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f), 0.0f, 0 });
	glm::vec4 colour = glm::vec4(1.0f, 1.0f, 0.5f, 0.5f);
	textures[0].push_back({ 0, 1, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ 0, -1, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ 1, 0, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ -1, 0, 1, 1, colour, 0.0f, 0 });
	colour = glm::vec4(1.0f, 1.0f, 0.5f, 0.3f);
	textures[0].push_back({ 1, 1, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ 1, -1, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ -1, 1, 1, 1, colour, 0.0f, 0 });
	textures[0].push_back({ -1, -1, 1, 1, colour, 0.0f, 0 });
	colour = glm::vec4(0.3f, 0.35f, 0.38f, 1.0f);
	// WALL
	textures[1].push_back({ 0, 3, 20, 14, colour, 4.0f, 0 });
	textures[1].push_back({ 3, 0, 14, 20, colour, 4.0f, 0 });
	textures[1].push_back({ 1, 1, 18, 18, glm::vec4(0.26f, 0.31f, 0.35f, 1.0f), 4.0f, 0 });
	textures[1].push_back({ 4, 4, 12, 12, glm::vec4(0.25f, 0.3f, 0.35f, 1.0f), 6.0f, 1 });
	// DOOR
	textures[2].push_back({ 0, 3, 20, 14, colour, 4.0f, 0 });
	textures[2].push_back({ 3, 0, 14, 20, colour, 4.0f, 0 });
	textures[2].push_back({ 1, 1, 18, 18, glm::vec4(0.26f, 0.31f, 0.35f, 1.0f), 4.0f, 0 });
	textures[2].push_back({ 3, 3, 14, 14, glm::vec4(0.23f, 0.27f, 0.3f, 1.0f), 3.0f, 1 });
	textures[2].push_back({ 5, 5, 10, 10, glm::vec4(0.25f, 0.3f, 0.35f, 1.0f), 6.0f, 1 });
	// DOOR (open)
	textures[3].push_back({ 0, 1, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 0, 6, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 0, 11, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 0, 16, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 19, 1, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 19, 6, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 19, 11, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 19, 16, 1, 3, colour, 4.0f, 0 });
	textures[3].push_back({ 1, 0, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 6, 0, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 11, 0, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 16, 0, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 1, 19, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 6, 19, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 11, 19, 3, 1, colour, 4.0f, 0 });
	textures[3].push_back({ 16, 19, 3, 1, colour, 4.0f, 0 });
	// MOVING_WALL
	textures[4].push_back({ 0, 3, 20, 14, colour, 4.0f, 0 });
	textures[4].push_back({ 3, 0, 14, 20, colour, 4.0f, 0 });
	textures[4].push_back({ 1, 1, 18, 18, glm::vec4(0.26f, 0.31f, 0.35f, 1.0f), 4.0f, 0 });
	textures[4].push_back({ 4, 4, 12, 12, glm::vec4(0.25f, 0.3f, 0.35f, 1.0f), 6.0f, 1 });
	colour = glm::vec4(0.23f, 0.27f, 0.3f, 1.0f);
	textures[4].push_back({ 3, 9, 14, 2, colour, 3.0f, 2 });
	textures[4].push_back({ 9, 3, 2, 14, colour, 3.0f, 2 });
	textures[4].push_back({ 4, 8, 1, 4, colour, 3.0f, 2 });
	textures[4].push_back({ 15, 8, 1, 4, colour, 3.0f, 2 });
	textures[4].push_back({ 8, 4, 4, 1, colour, 3.0f, 2 });
	textures[4].push_back({ 8, 15, 4, 1, colour, 3.0f, 2 });
	textures[4].push_back({ 5, 7, 1, 6, colour, 3.0f, 2 });
	textures[4].push_back({ 14, 7, 1, 6, colour, 3.0f, 2 });
	textures[4].push_back({ 7, 5, 6, 1, colour, 3.0f, 2 });
	textures[4].push_back({ 7, 14, 6, 1, colour, 3.0f, 2 });
	// MIRROR (N/S)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	textures[5].push_back({ 1, 8, 18, 4, colour, 4.0f, 3 });
	textures[5].push_back({ 0, 9, 20, 2, colour, 4.0f, 3 });
	textures[5].push_back({ 1, 9, 16, 2, glm::vec4(0.7f, 0.95f, 1.0f, 1.0f), 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[5].push_back({ 3, 10, 14, 1, colour, 4.0f, 4 });
	textures[5].push_back({ 17, 9, 2, 2, colour, 4.0f, 4 });
	textures[5].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (NNW/SSE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 5; i++)
		textures[6].push_back({ 2 + 3 * i, 6 + i, 4, 4, colour, 4.0f, 3 });
	textures[6].push_back({ 1, 5, 2, 4, colour, 4.0f, 3 });
	textures[6].push_back({ 0, 7, 1, 2, colour, 4.0f, 3 });
	textures[6].push_back({ 17, 11, 2, 4, colour, 4.0f, 3 });
	textures[6].push_back({ 19, 11, 1, 2, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	textures[6].push_back({ 1, 7, 5, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 6, 8, 3, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 8, 9, 4, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 12, 10, 3, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 15, 11, 3, 1, colour, 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[6].push_back({ 2, 8, 4, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 5, 9, 3, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 8, 10, 4, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 11, 11, 3, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 14, 12, 5, 1, colour, 4.0f, 4 });
	textures[6].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (NW/SE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 13; i++)
		textures[7].push_back({ 3 + i, 2 + i, 2, 4, colour, 0.0f, 3 });
	textures[7].push_back({ 2, 3, 1, 2, colour, 4.0f, 3 });
	textures[7].push_back({ 17, 15, 1, 2, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 13; i++)
		textures[7].push_back({ 3 + i, 3 + i, 2, 2, colour, 0.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 9; i++)
		textures[7].push_back({ 4 + i, 5 + i, 1, 1, colour, 0.0f, 4 });
	for (int i = 0; i < 3; i++)
		textures[7].push_back({ 13 + i, 14 + i, 2, 1, colour, 4.0f, 4 });
	textures[7].push_back({ 11, 11, 1, 1, colour, 4.0f, 4 });
	textures[7].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (NWW/SEE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 5; i++)
		textures[8].push_back({ 6 + i, 2 + 3 * i, 4, 4, colour, 4.0f, 3 });
	textures[8].push_back({ 5, 1, 4, 2, colour, 4.0f, 3 });
	textures[8].push_back({ 7, 0, 2, 1, colour, 4.0f, 3 });
	textures[8].push_back({ 11, 17, 4, 2, colour, 4.0f, 3 });
	textures[8].push_back({ 11, 19, 2, 1, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	textures[8].push_back({ 8, 2, 1, 4, colour, 4.0f, 4 });
	textures[8].push_back({ 9, 5, 1, 3, colour, 4.0f, 4 });
	textures[8].push_back({ 10, 8, 1, 4, colour, 4.0f, 4 });
	textures[8].push_back({ 11, 11, 1, 3, colour, 4.0f, 4 });
	textures[8].push_back({ 12, 14, 1, 5, colour, 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[8].push_back({ 7, 1, 1, 5, colour, 4.0f, 4 });
	textures[8].push_back({ 8, 6, 1, 3, colour, 4.0f, 4 });
	textures[8].push_back({ 9, 8, 1, 4, colour, 4.0f, 4 });
	textures[8].push_back({ 10, 12, 1, 3, colour, 4.0f, 4 });
	textures[8].push_back({ 11, 15, 1, 3, colour, 4.0f, 4 });
	textures[8].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (W/E)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	textures[9].push_back({ 8, 1, 4, 18, colour, 4.0f, 3 });
	textures[9].push_back({ 9, 0, 2, 20, colour, 4.0f, 3 });
	textures[9].push_back({ 9, 1, 2, 16, glm::vec4(0.8f, 0.95f, 1.0f, 1.0f), 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[9].push_back({ 10, 3, 1, 14, colour, 4.0f, 4 });
	textures[9].push_back({ 9, 17, 2, 2, colour, 4.0f, 4 });
	textures[9].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (SWW/NEE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 5; i++)
		textures[10].push_back({ 10 - i, 2 + 3 * i, 4, 4, colour, 4.0f, 3 });
	textures[10].push_back({ 11, 1, 4, 2, colour, 4.0f, 3 });
	textures[10].push_back({ 11, 0, 2, 1, colour, 4.0f, 3 });
	textures[10].push_back({ 5, 17, 4, 2, colour, 4.0f, 3 });
	textures[10].push_back({ 7, 19, 2, 1, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	textures[10].push_back({ 11, 2, 1, 4, colour, 4.0f, 4 });
	textures[10].push_back({ 10, 5, 1, 3, colour, 4.0f, 4 });
	textures[10].push_back({ 9, 8, 1, 4, colour, 4.0f, 4 });
	textures[10].push_back({ 8, 11, 1, 3, colour, 4.0f, 4 });
	textures[10].push_back({ 7, 14, 1, 5, colour, 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[10].push_back({ 12, 1, 1, 5, colour, 4.0f, 4 });
	textures[10].push_back({ 11, 6, 1, 3, colour, 4.0f, 4 });
	textures[10].push_back({ 10, 8, 1, 4, colour, 4.0f, 4 });
	textures[10].push_back({ 9, 12, 1, 3, colour, 4.0f, 4 });
	textures[10].push_back({ 8, 15, 1, 3, colour, 4.0f, 4 });
	textures[10].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (SW/NE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 13; i++)
		textures[11].push_back({ 15 - i, 2 + i, 2, 4, colour, 0.0f, 3 });
	textures[11].push_back({ 17, 3, 1, 2, colour, 4.0f, 3 });
	textures[11].push_back({ 2, 15, 1, 2, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 13; i++)
		textures[11].push_back({ 15 - i, 3 + i, 2, 2, colour, 0.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 9; i++)
		textures[11].push_back({ 15 - i, 5 + i, 1, 1, colour, 0.0f, 4 });
	for (int i = 0; i < 3; i++)
		textures[11].push_back({ 5 - i, 14 + i, 2, 1, colour, 4.0f, 4 });
	textures[11].push_back({ 8, 11, 1, 1, colour, 4.0f, 4 });
	textures[11].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// MIRROR (SSW/NNE)
	colour = glm::vec4(0.5f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 5; i++)
		textures[12].push_back({ 14 - 3 * i, 6 + i, 4, 4, colour, 4.0f, 3 });
	textures[12].push_back({ 17, 5, 2, 4, colour, 4.0f, 3 });
	textures[12].push_back({ 19, 7, 1, 2, colour, 4.0f, 3 });
	textures[12].push_back({ 1, 11, 2, 4, colour, 4.0f, 3 });
	textures[12].push_back({ 0, 11, 1, 2, colour, 4.0f, 3 });
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	textures[12].push_back({ 14, 7, 5, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 11, 8, 3, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 8, 9, 4, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 5, 10, 3, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 2, 11, 3, 1, colour, 4.0f, 4 });
	colour = glm::vec4(0.9f, 0.95f, 1.0f, 1.0f);
	textures[12].push_back({ 14, 8, 4, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 12, 9, 3, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 8, 10, 4, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 6, 11, 3, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 1, 12, 5, 1, colour, 4.0f, 4 });
	textures[12].push_back({ 9, 9, 2, 2, glm::vec4(0.6f, 0.6f, 0.6f, 1.0f), 5.0f, 4 });
	// DIAGONAL_MIRROR (NW/SE)
	for (int i = 0; i < 17; i++)
		textures[13].push_back({ 1 + i, 1 + i, 2, 2, glm::vec4(0.6f, 0.95f, 1.0f, 1.0f), 0.0f, 4 });
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[13].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[13].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[13].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[13].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 18; i++)
		textures[13].push_back({ 1 + i, 1 + i, 1, 1, glm::vec4(0.9f, 0.95f, 1.0f, 1.0f), 0.0f, 5 });
	// DIAGONAL_MIRROR (NE/SW)
	for (int i = 0; i < 17; i++)
		textures[14].push_back({ 17 - i, 1 + i, 2, 2, glm::vec4(0.6f, 0.95f, 1.0f, 1.0f), 0.0f, 4 });
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[14].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[14].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[14].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[14].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 18; i++)
		textures[14].push_back({ 18 - i, 1 + i, 1, 1, glm::vec4(0.9f, 0.95f, 1.0f, 1.0f), 0.0f, 5 });
	// MIRROR_BLOCK
	textures[15].push_back({ 0, 0, 20, 20, glm::vec4(0.5f, 0.95f, 1.0f, 1.0f), 4.0f, 3 });
	textures[15].push_back({ 1, 1, 18, 18, glm::vec4(0.7f, 0.95f, 1.0f, 1.0f), 4.0f, 3 });
	for (int i = 0; i < 9; i++)
		textures[15].push_back({ 18 - i, 10 + i, 1 + i, 1, colour, 0.0f, 4 });
	for (int i = 0; i < 13; i++)
		textures[15].push_back({ 18 - i, 2 + i, 1, 4, colour, 0.0f, 4 });
	for (int i = 0; i < 4; i++) {
		textures[15].push_back({ 1, 1 + i, 4 - i, 1, colour, 4.0f, 4 });
		textures[15].push_back({ 2 + i, 17 - i, 4 - i, 1, colour, 4.0f, 4 });
	}
	colour = glm::vec4(0.8f, 0.95f, 1.0f, 1.0f);
	for (int i = 0; i < 4; i++)
		textures[15].push_back({ 14 + i, 18 - i, 2, 1, colour, 4.0f, 4 });
	textures[15].push_back({ 18, 14, 1, 1, colour, 4.0f, 4 });
	textures[15].push_back({ 1, 8, 2, 4, colour, 4.0f, 4 });
	textures[15].push_back({ 3, 7, 1, 4, colour, 4.0f, 4 });
	// MIRROR_DOOR
	textures[16].push_back({ 0, 0, 20, 20, glm::vec4(0.5f, 0.95f, 1.0f, 1.0f), 4.0f, 3 });
	textures[16].push_back({ 1, 1, 18, 18, glm::vec4(0.7f, 0.95f, 1.0f, 1.0f), 4.0f, 3 });
	for (int i = 0; i < 9; i++)
		textures[16].push_back({ 18 - i, 10 + i, 1 + i, 1, colour, 0.0f, 4 });
	for (int i = 0; i < 13; i++)
		textures[16].push_back({ 18 - i, 2 + i, 1, 4, colour, 0.0f, 4 });
	for (int i = 0; i < 4; i++) {
		textures[16].push_back({ 1, 1 + i, 4 - i, 1, colour, 4.0f, 4 });
		textures[16].push_back({ 2 + i, 17 - i, 4 - i, 1, colour, 4.0f, 4 });
	}
	colour = glm::vec4(0.5f, 0.8f, 0.9f, 1.0f);
	textures[16].push_back({ 3, 3, 14, 2, colour, 4.0f, 4 });
	textures[16].push_back({ 3, 5, 2, 12, colour, 4.0f, 4 });
	textures[16].push_back({ 15, 5, 2, 12, colour, 4.0f, 4 });
	textures[16].push_back({ 5, 15, 10, 2, colour, 4.0f, 4 });
	// MIRROR_DOOR (open)
	colour = glm::vec4(0.7f, 0.95f, 1.0f, 1.0f);
	textures[17].push_back({ 0, 1, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 0, 6, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 0, 11, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 0, 16, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 19, 1, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 19, 6, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 19, 11, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 19, 16, 1, 3, colour, 4.0f, 0 });
	textures[17].push_back({ 1, 0, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 6, 0, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 11, 0, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 16, 0, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 1, 19, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 6, 19, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 11, 19, 3, 1, colour, 4.0f, 0 });
	textures[17].push_back({ 16, 19, 3, 1, colour, 4.0f, 0 });
	// GLASS_BLOCK (N/S)
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[18].push_back({ 1, 6, 18, 1, colour, 5.0f, 5 });
	textures[18].push_back({ 1, 13, 18, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.3f, 0.4f, 0.35f, 0.5f);
	textures[18].push_back({ 1, 7, 1, 1, colour, 5.0f, 5 });
	textures[18].push_back({ 1, 12, 1, 1, colour, 5.0f, 5 });
	textures[18].push_back({ 18, 7, 1, 1, colour, 5.0f, 5 });
	textures[18].push_back({ 18, 12, 1, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.2f, 0.25f, 0.22f, 0.8f);
	textures[18].push_back({ 0, 7, 1, 6, colour, 5.0f, 5 });
	textures[18].push_back({ 19, 7, 1, 6, colour, 5.0f, 5 });
	textures[18].push_back({ 9, 9, 2, 2, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), 5.0f, 5 });
	// GLASS_BLOCK (NW/SE)
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	for (int i = 0; i < 14; i++) {
		textures[19].push_back({ 1 + i, 5 + i, 1, 1, colour, 0.0f, 5 });
		textures[19].push_back({ 5 + i, 1 + i, 1, 1, colour, 0.0f, 5 });
	}
	colour = glm::vec4(0.25f, 0.3f, 0.27f, 0.7f);
	textures[19].push_back({ 1, 4, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 2, 5, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 4, 1, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 5, 2, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 14, 17, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 15, 18, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 17, 14, 1, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 18, 15, 1, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.2f, 0.25f, 0.22f, 0.8f);
	textures[19].push_back({ 2, 3, 1, 2, colour, 5.0f, 5 });
	textures[19].push_back({ 3, 2, 2, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 15, 17, 2, 1, colour, 5.0f, 5 });
	textures[19].push_back({ 17, 15, 1, 2, colour, 5.0f, 5 });
	textures[19].push_back({ 9, 9, 2, 2, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), 5.0f, 5 });
	// GLASS_BLOCK (W/E)
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[20].push_back({ 6, 1, 1, 18, colour, 5.0f, 5 });
	textures[20].push_back({ 13, 1, 1, 18, colour, 5.0f, 5 });
	colour = glm::vec4(0.25f, 0.3f, 0.27f, 0.7f);
	textures[20].push_back({ 7, 1, 1, 1, colour, 5.0f, 5 });
	textures[20].push_back({ 12, 1, 1, 1, colour, 5.0f, 5 });
	textures[20].push_back({ 7, 18, 1, 1, colour, 5.0f, 5 });
	textures[20].push_back({ 12, 18, 1, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.2f, 0.25f, 0.22f, 0.8f);
	textures[20].push_back({ 7, 0, 6, 1, colour, 5.0f, 5 });
	textures[20].push_back({ 7, 19, 6, 1, colour, 5.0f, 5 });
	textures[20].push_back({ 9, 9, 2, 2, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), 5.0f, 5 });
	// GLASS_BLOCK (SW/NE)
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	for (int i = 0; i < 14; i++) {
		textures[21].push_back({ 18 - i, 5 + i, 1, 1, colour, 0.0f, 5 });
		textures[21].push_back({ 14 - i, 1 + i, 1, 1, colour, 0.0f, 5 });
	}
	colour = glm::vec4(0.25f, 0.3f, 0.27f, 0.7f);
	textures[21].push_back({ 18, 4, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 17, 5, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 15, 1, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 14, 2, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 5, 17, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 4, 18, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 2, 14, 1, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 1, 15, 1, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.2f, 0.25f, 0.22f, 0.8f);
	textures[21].push_back({ 17, 3, 1, 2, colour, 5.0f, 5 });
	textures[21].push_back({ 15, 2, 2, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 3, 17, 2, 1, colour, 5.0f, 5 });
	textures[21].push_back({ 2, 15, 1, 2, colour, 5.0f, 5 });
	textures[21].push_back({ 9, 9, 2, 2, glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), 5.0f, 5 });
	// FIXED_BLOCK
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[22].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[22].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[22].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[22].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 4; i++) {
		textures[22].push_back({ 2 + i, 5 - i, 1, 1, colour, 5.0f, 5 });
		textures[22].push_back({ 14 + i, 17 - i, 1, 1, colour, 5.0f, 5 });
	}
	// MOVING_BLOCK
	textures[23].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[23].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[23].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[23].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 4; i++) {
		textures[23].push_back({ 2 + i, 5 - i, 1, 1, colour, 5.0f, 5 });
		textures[23].push_back({ 14 + i, 17 - i, 1, 1, colour, 5.0f, 5 });
	}
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.1f);
	textures[23].push_back({ 6, 9, 3, 2, colour, 3.0f, 6 });
	textures[23].push_back({ 11, 9, 3, 2, colour, 3.0f, 6 });
	textures[23].push_back({ 9, 6, 2, 8, colour, 3.0f, 6 });
	textures[23].push_back({ 3, 9, 1, 2, colour, 3.0f, 6 });
	textures[23].push_back({ 16, 9, 1, 2, colour, 3.0f, 6 });
	textures[23].push_back({ 9, 3, 2, 1, colour, 3.0f, 6 });
	textures[23].push_back({ 9, 16, 2, 1, colour, 3.0f, 6 });
	textures[23].push_back({ 4, 8, 1, 4, colour, 3.0f, 6 });
	textures[23].push_back({ 15, 8, 1, 4, colour, 3.0f, 6 });
	textures[23].push_back({ 8, 4, 4, 1, colour, 3.0f, 6 });
	textures[23].push_back({ 8, 15, 4, 1, colour, 3.0f, 6 });
	textures[23].push_back({ 5, 7, 1, 6, colour, 3.0f, 6 });
	textures[23].push_back({ 14, 7, 1, 6, colour, 3.0f, 6 });
	textures[23].push_back({ 7, 5, 6, 1, colour, 3.0f, 6 });
	textures[23].push_back({ 7, 14, 6, 1, colour, 3.0f, 6 });
	// PRISM
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.15f);
	textures[24].push_back({ 1, 2, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 18, 2, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 2, 4, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 17, 4, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 3, 6, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 16, 6, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 4, 8, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 15, 8, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 4, 10, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 15, 10, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 6, 11, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 13, 11, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 7, 13, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 12, 13, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 8, 15, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 11, 15, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 9, 17, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 10, 17, 1, 1, colour, 5.0f, 5 });
	colour = glm::vec4(0.8f, 1.0f, 0.9f, 0.2f);
	textures[24].push_back({ 0, 1, 20, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 0, 2, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 19, 2, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 1, 3, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 18, 3, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 2, 5, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 17, 5, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 3, 7, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 16, 7, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 4, 9, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 15, 9, 1, 1, colour, 5.0f, 5 });
	textures[24].push_back({ 5, 10, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 14, 10, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 6, 12, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 13, 12, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 7, 14, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 12, 14, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 8, 16, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 11, 16, 1, 2, colour, 5.0f, 5 });
	textures[24].push_back({ 9, 18, 2, 1, colour, 5.0f, 5 });
	// SPDC_CRYSTAL
	textures[25].push_back({ 1, 1, 19, 19, glm::vec4(0.8f, 0.8f, 0.6f, 0.1f), 5.0f, 5 });
	colour = glm::vec4(0.8f, 0.8f, 0.6f, 0.2f);
	textures[25].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[25].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[25].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[25].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 4; i++) {
		textures[25].push_back({ 2 + i, 5 - i, 1, 1, colour, 5.0f, 5 });
		textures[25].push_back({ 14 + i, 17 - i, 1, 1, colour, 5.0f, 5 });
	}
	// MOVING_CRYSTAL
	textures[26].push_back({ 1, 1, 19, 19, glm::vec4(0.8f, 0.8f, 0.6f, 0.1f), 5.0f, 5 });
	textures[26].push_back({ 0, 0, 20, 1, colour, 5.0f, 5 });
	textures[26].push_back({ 0, 1, 1, 19, colour, 5.0f, 5 });
	textures[26].push_back({ 1, 19, 19, 1, colour, 5.0f, 5 });
	textures[26].push_back({ 19, 1, 1, 18, colour, 5.0f, 5 });
	for (int i = 0; i < 4; i++) {
		textures[26].push_back({ 2 + i, 5 - i, 1, 1, colour, 5.0f, 5 });
		textures[26].push_back({ 14 + i, 17 - i, 1, 1, colour, 5.0f, 5 });
	}
	colour = glm::vec4(0.7f, 0.7f, 0.5f, 0.3f);
	textures[26].push_back({ 6, 9, 3, 2, colour, 3.0f, 6 });
	textures[26].push_back({ 11, 9, 3, 2, colour, 3.0f, 6 });
	textures[26].push_back({ 9, 6, 2, 8, colour, 3.0f, 6 });
	textures[26].push_back({ 3, 9, 1, 2, colour, 3.0f, 6 });
	textures[26].push_back({ 16, 9, 1, 2, colour, 3.0f, 6 });
	textures[26].push_back({ 9, 3, 2, 1, colour, 3.0f, 6 });
	textures[26].push_back({ 9, 16, 2, 1, colour, 3.0f, 6 });
	textures[26].push_back({ 4, 8, 1, 4, colour, 3.0f, 6 });
	textures[26].push_back({ 15, 8, 1, 4, colour, 3.0f, 6 });
	textures[26].push_back({ 8, 4, 4, 1, colour, 3.0f, 6 });
	textures[26].push_back({ 8, 15, 4, 1, colour, 3.0f, 6 });
	textures[26].push_back({ 5, 7, 1, 6, colour, 3.0f, 6 });
	textures[26].push_back({ 14, 7, 1, 6, colour, 3.0f, 6 });
	textures[26].push_back({ 7, 5, 6, 1, colour, 3.0f, 6 });
	textures[26].push_back({ 7, 14, 6, 1, colour, 3.0f, 6 });
	// SPLITTER (NW/SE)

	// SPLITTER (NE/SW)

	// BOMB

	// SENSOR

}
