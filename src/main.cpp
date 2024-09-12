#include <algorithm>
#include <chrono>
#include <filesystem>
#include <thread>
#include <ctime>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "head.hpp"

#include "gui.hpp"

#ifdef _WIN32
extern "C" {
	__declspec(dllexport) unsigned NvOptimusEnablement = 1;
	__declspec(dllexport) unsigned AmdPowerXpressRequestHighPerformance = 1;
}
#endif

double nano(void);
void move_cb(GLFWwindow* window, double xpos, double ypos);
void click_cb(GLFWwindow* window, int button, int action, int mods);
void text_cb(GLFWwindow* window, unsigned codepoint);
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

GLFWwindow* window;

int main(void) {
	srand((unsigned)time(NULL));
	//if (!std::filesystem::exists(std::filesystem::path("./saves")))
	//	std::filesystem::create_directory(std::filesystem::path("./saves"));
	
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(1280, 720, "Photon", NULL, NULL);
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
	res::load_vao();
	gui::load_font();
	gui::load_gui();
	res::loader::load_tex();
	res::shaders::load();
	glfwSetCursorPosCallback(window, move_cb);
	glfwSetMouseButtonCallback(window, click_cb);
	glfwSetCharCallback(window, text_cb);
	glfwSetKeyCallback(window, key_cb);
	double next = nano() / 1.0E9;
	int skipped = 1;
	constexpr const std::chrono::duration<double> zero(0);
	int tps = 64;
	// This program is probably about as stable as the economy
	// of the Weimar Republic during the hyperinflation of 1923
	while (!glfwWindowShouldClose(window)) {
		if (gui::quit)
			break;
		double cur = nano() / 1.0E9;
		if (cur - next > 0.5)
			next = cur;
		if (cur >= next) {
			if (gui::menu)
				next += 1.0 / tps;
			else {
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
			}
			if (cur < next || skipped > 5) {
				glClear(GL_COLOR_BUFFER_BIT);
				if (gui::menu) {
					for (int i = 0; i < gui::elements.size(); i++)
						gui::elements.data()[i]->render();
				}
				else {
					for (int layer = 0; layer < 7; layer++)
						for (int i = 0; i < res::objects.size(); i++)
							res::objects.data()[i].render(layer);
					object::selected->border();
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
						it->render();
				}
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
	for (int i = 0; i < gui::elements.size(); i++)
		delete gui::elements.data()[i];
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

double nano(void) {
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void move_cb(GLFWwindow* window, double xpos, double ypos) {
	if (!gui::menu)
		return;
	if (xpos < 0.0 || ypos < 0.0)
		return;
	for (int i = 0; i < gui::elements.size(); i++)
		gui::elements.data()[i]->handler(gui::enum_event::MOVE, (unsigned)xpos, (unsigned)ypos, 0);
}

void click_cb(GLFWwindow* window, int button, int action, int mods) {
	if (!gui::menu)
		return;
	for (int i = 0; i < gui::elements.size(); i++)
		gui::elements.data()[i]->handler(gui::enum_event::CLICK, button, action, mods);
}

void text_cb(GLFWwindow* window, unsigned codepoint) {
	if (!gui::menu)
		return;
	if (codepoint == '\f' || codepoint == '\n' || codepoint == '\r'
		|| codepoint == '\t' || codepoint == '\v')
		return;
	for (int i = 0; i < gui::elements.size(); i++)
		gui::elements.data()[i]->handler(gui::enum_event::TEXT, codepoint, 0, 0);
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (gui::menu) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			int index = 0;
			for (; index < gui::elements.size() && gui::elements.data()[index]->text != "<BACK"; index++);
			if (gui::elements.data()[index]->visible) {
				for (int i = 1; i < index; i++)
					gui::elements[i]->visible = true;
				for (int i = index; i < index + 18; i++)
					gui::elements[i]->visible = false;
			}
			else if (gui::started)
				gui::menu = false;
		}
		else
			for (int i = 0; i < gui::elements.size(); i++)
				gui::elements.data()[i]->handler(gui::enum_event::KEY, key, action, mods);
		return;
	}
	if (action != GLFW_PRESS)
		return;
	if (key == keybinds::up || key == keybinds::left
		|| key == keybinds::down || key == keybinds::right)
		select(key);
	else if (key == keybinds::ccw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			int step = object::selected->type == object::enum_type::MIRROR ? 1 : 2;
			object::selected->orientation = (object::enum_orientation)(dir == 16 - step ? 0 : dir + step);
		}
	}
	else if (key == keybinds::cw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			int step = object::selected->type == object::enum_type::MIRROR ? 1 : 2;
			object::selected->orientation = (object::enum_orientation)(dir == 0 ? 16 - step : dir - step);
		}
	}
	else if (key == keybinds::perp) {
		if (object::selected) {
			int dir = (int)object::selected->orientation + 4;
			if (dir > 15)
				dir -= 16;
			object::selected->orientation = (object::enum_orientation)dir;
		}
	}
	else if (key == keybinds::toggle) {
		if (object::selected && !object::selected->moving) {
			object::selected->toggle = !object::selected->toggle;
			if (object::selected->type == object::enum_type::MOVING_WALL
				|| object::selected->type == object::enum_type::MOVING_BLOCK
				|| object::selected->type == object::enum_type::MOVING_CRYSTAL)
				object::selected->moving = true;
		}
	}
	else if (key == GLFW_KEY_ESCAPE)
		gui::menu = true;
}
