#include <algorithm>
#include <chrono>
#include <thread>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "head.hpp"
#include "logic.hpp"

#ifdef _WIN32
extern "C" {
	__declspec(dllexport) unsigned NvOptimusEnablement = 1;
	__declspec(dllexport) unsigned AmdPowerXpressRequestHighPerformance = 1;
}
#endif

double nano(void);
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(void) {
	srand(time(NULL));
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Photon", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glViewport(0, 0, 1280, 720);
	res::init_vao();
	res::loader::init();
	res::shaders::init();
	res::objects.push_back(object(40.0, 40.0, 20, 20, object::enum_orientation::NONE,
		object::enum_type::MOVING_WALL,
		&res::loader::textures[4], NULL));
	object::selected = &res::objects[0];
	object::selected->x2 = 80;
	object::selected->y2 = 80;
	glfwSetKeyCallback(window, key_cb);
	double next = nano() / 1.0E9;
	int skipped = 1;
	constexpr const std::chrono::duration<double> zero(0);
	int tps = 64;
	// This program is probably about as stable as the economy
	// of the Weimar Republic during the hyperinflation of 1923
	while (!glfwWindowShouldClose(window)) {
		double cur = nano() / 1.0E9;
		if (cur - next > 0.5)
			next = cur;
		if (cur >= next) {
			double len = 1.0;
			for (int i = 0; i < res::objects.size(); i++)
				if (res::objects.data()[i].moving)
					res::objects.data()[i].tick(tps);
			for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
				it->pre_tick(tps);
				if (it->_tick < len && it->_tick != 0)
					len = it->_tick;
			}
			for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();)
				it->tick(tps, len, &it);
			for (int i = 0; i < photon::removing.size(); i++)
				photon::removing.data()[i]->destroy();
			if (photon::removing.size() > 0) {
				for (std::list<node>::iterator it = photon::nodes.begin(); it != photon::nodes.end();) {
					node* n = &*it;
					if (std::any_of(photon::removing.begin(), photon::removing.end(),
						[n](node* other) { return other == n; }))
						it = photon::nodes.erase(it);
					else
						++it;
				}
				photon::removing.clear();
			}
			next += len / tps;
			if (cur < next || skipped > 5) {
				glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				for (int i = 0; i < res::objects.size(); i++)
					res::objects.data()[i].render();
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
					it->render();
				glfwSwapBuffers(window);
				skipped = 1;
			}
			else
				skipped++;
		}
		else {
			double now = nano();
			while (nano() - now < (next - cur) * 1.0E9)
				std::this_thread::sleep_for(zero);
		}
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

double nano(void) {
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	switch (key) {
	case GLFW_KEY_W:
	case GLFW_KEY_A:
	case GLFW_KEY_S:
	case GLFW_KEY_D:
		select(key);
		break;
	case GLFW_KEY_LEFT_BRACKET:
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 0 ? 15 : dir - 1);
		}
		break;
	case GLFW_KEY_RIGHT_BRACKET:
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 15 ? 0 : dir + 1);
		}
		break;
	case GLFW_KEY_BACKSLASH:
		if (object::selected) {
			int dir = (int)object::selected->orientation + 8;
			if (dir > 15)
				dir -= 16;
			object::selected->orientation = (object::enum_orientation)dir;
		}
		break;
	case GLFW_KEY_SLASH:
		if (object::selected) {
			object::selected->toggle = !object::selected->toggle;
			if (object::selected->type == object::enum_type::MOVING_WALL
				|| object::selected->type == object::enum_type::MOVING_BLOCK
				|| object::selected->type == object::enum_type::MOVING_CRYSTAL)
				object::selected->moving = true;
		}
		break;
	default:
		break;
	}
	
}
