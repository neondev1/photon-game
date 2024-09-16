#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "head.hpp"

#include "gui.hpp"

namespace fs = std::filesystem;

#ifdef _WIN32
extern "C" {
	__declspec(dllexport) unsigned NvOptimusEnablement = 1;
	__declspec(dllexport) unsigned AmdPowerXpressRequestHighPerformance = 1;
}
#endif

void move_cb(GLFWwindow* window, double xpos, double ypos);
void click_cb(GLFWwindow* window, int button, int action, int mods);
void text_cb(GLFWwindow* window, unsigned codepoint);
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

GLFWwindow* gui::window;

int main(void) {
	srand((unsigned)time(NULL));
	if (fs::is_regular_file(fs::path("./settings"))) {
		std::ifstream in("./settings");
		in >> keybinds::up >> keybinds::left >> keybinds::down >> keybinds::right;
		in >> keybinds::ccw >> keybinds::cw >> keybinds::perp >> keybinds::toggle;
		in >> gamestate::save;
		in.close();
	}
	else {
		std::ofstream out("./settings");
		out << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
		out << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle;
		out << std::endl;
		out.close();
	}
	if (!gamestate::save.empty() && fs::is_regular_file(gamestate::save)) {
		std::ifstream in(gamestate::save);
		char hc;
		in >> hc;
		gamestate::hardcore = hc == 'h';
		in >> gamestate::level >> gamestate::failures >> gamestate::time;
		if (in.fail())
			gamestate::save.clear();
		in.close();
	}
	else
		gamestate::save.clear();
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	gui::window = glfwCreateWindow(1280, 720, "Photon", NULL, NULL);
	if (gui::window == NULL) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(gui::window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(gui::window);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glViewport(0, 0, 1280, 720);
	res::load_vao();
	gui::load_font();
	gui::load_gui();
	res::loader::load_tex();
	res::shaders::load();
	glfwSetCursorPosCallback(gui::window, move_cb);
	glfwSetMouseButtonCallback(gui::window, click_cb);
	glfwSetCharCallback(gui::window, text_cb);
	glfwSetKeyCallback(gui::window, key_cb);
	double next = glfwGetTime();
	int skipped = 1;
	constexpr const std::chrono::duration<double> zero(0);
	int tps = 64;
	size_t frame = 0;
	// This program is probably about as stable as the economy
	// of the Weimar Republic during the hyperinflation of 1923
	while (!glfwWindowShouldClose(gui::window)) {
		if (gui::quit)
			break;
		double cur = glfwGetTime();
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
				int fails = gamestate::failures;
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();)
					it->tick(tps, len, &it);
				if (gamestate::failures == fails) {
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
				else if (gamestate::hardcore) {
					photon::removing.clear();
					photon::nodes.clear();
					photon::photons.clear();
					object::selected = NULL;
					object::invalidated.clear();
					res::objects.clear();
					std::ofstream out(gamestate::save, std::ios::trunc);
					out << (gamestate::hardcore ? 'h' : 'n') << '\n' << gamestate::level << '\n';
					out << gamestate::failures << '\n' << gamestate::time << std::endl;
					out.close();
					gamestate::started = false;
					gui::elements[1]->text = "GAME OVER";
					int index = 0;
					for (; index < gui::elements.size() - 2
						&& gui::elements.data()[index]->text.find("Levels: ") == std::string::npos; index++);
					gui::elements.data()[index]->text = std::string("Levels: ") + std::to_string(gamestate::level);
					gui::elements.data()[index]->visible = true;
					gui::elements.data()[index + 1]->text = std::string("Time: ") + gui::time(gamestate::time);
					gui::elements.data()[index + 1]->visible = true;
				}

			}
			if (cur < next || skipped > 5) {
				frame++;
				if (gui::menu) {
					glClear(GL_COLOR_BUFFER_BIT);
					for (int i = 0; i < gui::elements.size(); i++)
						gui::elements.data()[i]->render();
				}
				else if (frame % 2 && !object::invalidate_all) {
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
						glEnable(GL_SCISSOR_TEST);
						glScissor(
							(GLint)round(it->_x - 240.0 / tps), (GLint)round(it->_y - 240.0 / tps),
							(GLsizei)(480.0 / tps), (GLsizei)(480.0 / tps));
						glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
							1, glm::value_ptr(res::objects.data()[0].texture->data()[0].colour));
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), res::objects.data()[0].texture->data()[0].noise);
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
						glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
							(GLfloat)(res::objects.data()[0].offset - round(it->_x - 240.0 / tps)),
							(GLfloat)(res::objects.data()[0].offset - round(it->_y - 240.0 / tps)));
						glUseProgram(res::shaders::rectangle);
						glBindVertexArray(res::rect_vao);
						glDrawArrays(GL_TRIANGLES, 0, 6);
						glBindVertexArray(0);
						glDisable(GL_SCISSOR_TEST);
					}
					for (int i = 1; i < res::objects.size(); i++) {
						object& obj = res::objects.data()[i];
						if (std::any_of(object::invalidated.begin(), object::invalidated.end(), [obj, tps](object* other) {
							return ((int)obj.type < (int)object::enum_type::GLASS_BLOCK
									|| (int)obj.type >(int)object::enum_type::FIXED_SPLITTER)
								&& distance(obj.midx(), obj.midy(), other->midx(), other->midy()) < 320.0 / tps;
							})
							|| std::any_of(photon::photons.begin(), photon::photons.end(), [obj, tps](photon p) {
							return ((int)obj.type < (int)object::enum_type::GLASS_BLOCK
									|| (int)obj.type >(int)object::enum_type::FIXED_SPLITTER)
								&& distance(obj.midx(), obj.midy(), p._x, p._y) < 320.0 / tps;
							}))
							obj.render(-1);
					}
					for (int i = 0; i < res::objects.size(); i++) {
						object& obj = res::objects.data()[i];
						if ((int)obj.type >= (int)object::enum_type::GLASS_BLOCK
							&& (int)obj.type <= (int)object::enum_type::FIXED_SPLITTER)
							obj.render(-1);
					}
					object::invalidated.clear();
					if (object::selected)
						object::selected->border();
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
						it->render();
				}
				else {
					object::invalidated.clear();
					object::invalidate_all = false;
					glClear(GL_COLOR_BUFFER_BIT);
					for (int layer = 0; layer < 7; layer++)
						for (int i = 0; i < res::objects.size(); i++)
							res::objects.data()[i].render(layer);
					if (object::selected)
						object::selected->border();
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
						it->render();
				}
				glfwSwapBuffers(gui::window);
				skipped = 1;
			}
			else
				skipped++;
		}
		else {
			double now = glfwGetTime();
			while (glfwGetTime() - now < next - cur)
				std::this_thread::sleep_for(zero);
		}
		glfwPollEvents();
	}
	if (!gamestate::save.empty()) {
		std::ofstream out(gamestate::save, std::ios::trunc);
		out << (gamestate::hardcore ? 'h' : 'n') << '\n' << gamestate::level << '\n';
		out << gamestate::failures << '\n' << gamestate::time << std::endl;
		out.close();
	}
	std::ofstream cfg("./settings", std::ios::trunc);
	cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
	cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
	cfg << gamestate::save << std::endl;
	cfg.close();
	for (int i = 0; i < gui::elements.size(); i++)
		delete gui::elements.data()[i];
	glfwDestroyWindow(gui::window);
	glfwTerminate();
	return EXIT_SUCCESS;
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
			bool flag = false;
			for (int i = 0; i < gui::elements.size(); i++) {
				if (dynamic_cast<gui::keybind_button*>(gui::elements.data()[i])
					&& dynamic_cast<gui::keybind_button*>(gui::elements.data()[i])->state == 2) {
					flag = true;
					break;
				}
			}
			if (flag)
				for (int i = 0; i < gui::elements.size(); i++)
					gui::elements.data()[i]->handler(gui::enum_event::KEY, key, action, mods);
			else {
				int index = 0;
				for (; index < gui::elements.size() && gui::elements.data()[index]->text != "<BACK"; index++);
				if (gui::elements.data()[index]->visible) {
					for (int i = 1; i < index; i++)
						gui::elements[i]->visible = true;
					for (int i = index; i < gui::elements.size(); i++)
						gui::elements[i]->visible = false;
				}
				else if (gamestate::started)
					gui::menu = false;
			}
		}
		else
			for (int i = 0; i < gui::elements.size(); i++)
				gui::elements.data()[i]->handler(gui::enum_event::KEY, key, action, mods);
		return;
	}
	if (action != GLFW_PRESS)
		return;
	int step;
	switch (object::selected->type) {
	case object::enum_type::MIRROR:			step = 1;	break;
	case object::enum_type::GLASS_BLOCK:	step = 2;	break;
	case object::enum_type::SPLITTER:		step = 4;	break;
	default:								step = 0;	break;
	}
	if (key == keybinds::up || key == keybinds::left
		|| key == keybinds::down || key == keybinds::right) {
		select(key);
		object::invalidate_all = true;
	}
	else if (key == keybinds::ccw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 16 - step ? 0 : dir + step);
			object::invalidated.push_back(object::selected);
		}
	}
	else if (key == keybinds::cw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 0 ? 16 - step : dir - step);
			object::invalidated.push_back(object::selected);
		}
	}
	else if (key == keybinds::perp) {
		if (object::selected) {
			int dir = (int)object::selected->orientation + 4;
			if (dir > 15)
				dir -= 16;
			object::selected->orientation = (object::enum_orientation)dir;
			object::invalidated.push_back(object::selected);
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
	else if (key == GLFW_KEY_ESCAPE) {
		std::ofstream out(gamestate::save, std::ios::trunc);
		out << (gamestate::hardcore ? 'h' : 'n') << '\n' << gamestate::level << '\n';
		out << gamestate::failures << '\n' << gamestate::time << std::endl;
		out.close();
		gui::menu = true;
	}
}
