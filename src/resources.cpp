#include <vector>

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
	float angle = rand(floor(gl_FragCoord.xy / vec2(pxsize) + offset));
	vec2 d = vec2(noise * cos(angle), noise * sin(angle));
	outcolour = vec4(colour);
	if (max(colour.r, max(colour.g, colour.b)) == colour.r)
		outcolour = vec4(colour.r, colour.g + d.x / 255.0, colour.b + d.y / 255.0, colour.a);
	if (max(colour.r, max(colour.g, colour.b)) == colour.g)
		outcolour = vec4(colour.r + d.y / 255.0, colour.g, colour.b + d.x / 255.0, colour.a);
	if (max(colour.r, max(colour.g, colour.b)) == colour.b)
		outcolour = vec4(colour.r + d.x / 255.0, colour.g + d.y / 255.0, colour.b, colour.a);
}
)""\0";
	}
}

void res::init_vao(void) {
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

void res::shaders::init(void) {
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
