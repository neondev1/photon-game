#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <thread>

#ifdef LOGIC_TEST
#	include <iostream>
#endif // LOGIC_TEST

#ifdef _WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif // WIN32_LEAN_AND_MEAN
#	ifndef VC_EXTRALEAN
#		define VC_EXTRALEAN
#	endif // VC_EXTRALEAN
#	include <windows.h>
#endif // _WIN32

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "head.hpp"
#include "gui.hpp"

namespace fs = std::filesystem;

#ifdef _WIN32
extern "C" {
#	ifdef _MSC_VER
	__declspec(dllexport) unsigned NvOptimusEnablement = 1;
	__declspec(dllexport) unsigned AmdPowerXpressRequestHighPerformance = 1;
#	elif defined(__GNUC__) || defined(__clang__)
	__attribute__((dllexport)) unsigned NvOptimusEnablement = 1;
	__attribute__((dllexport)) unsigned AmdPowerXpressRequestHighPerformance = 1;
#	endif // _MSC_VER, defined(__GNUC__) || defined(__clang__)
}
#endif // _WIN32

void move_cb(GLFWwindow* window, double xpos, double ypos);
void click_cb(GLFWwindow* window, int button, int action, int mods);
void text_cb(GLFWwindow* window, unsigned codepoint);
void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

// Debugging function
#ifdef LOGIC_TEST
static bool list_nodes(node* root, int depth);
#endif // LOGIC_TEST

long long game::frame = -1;

int main(void) {
	if (!glfwInit()) {
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to initialize GLFW", "", MB_OK | MB_ICONERROR);
#endif // _WIN32
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	gui::window = glfwCreateWindow(1280, 720, "Photon", nullptr, nullptr);
	if (gui::window == nullptr) {
		glfwTerminate();
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to create window", "", MB_OK | MB_ICONERROR);
#endif // _WIN32
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(gui::window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(gui::window);
		glfwTerminate();
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to load OpenGL", "", MB_OK | MB_ICONERROR);
#endif // _WIN32
		exit(EXIT_FAILURE);
	}
	glViewport(0, 0, 1280, 720);
	if (fs::is_regular_file("./settings")) {
		std::ifstream in("./settings");
		in >> keybinds::up >> keybinds::left >> keybinds::down >> keybinds::right;
		in >> keybinds::ccw >> keybinds::cw >> keybinds::perp >> keybinds::toggle;
		in >> keybinds::hint >> game::save;
		in.close();
	}
	else {
		std::ofstream out("./settings");
		out << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
		out << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
		out << keybinds::hint << std::endl;
		out.close();
	}
	if (!game::save.empty() && fs::is_regular_file(game::save)) {
		std::ifstream in(game::save);
		char hc;
		in >> hc;
		game::hardcore = hc == 'h';
		in >> game::level >> game::failures >> game::time;
		if (in.fail())
			game::save.clear();
		in.close();
	}
	else
		game::save.clear();
	res::load_vao();
	gui::load_font();
	gui::load_gui();
	res::loader::load_tex();
	res::loader::load_hbx();
	if (!res::loader::load_from_file("./levels"))
		res::loader::load_default();
	res::shaders::load();
	glfwSetCursorPosCallback(gui::window, move_cb);
	glfwSetMouseButtonCallback(gui::window, click_cb);
	glfwSetCharCallback(gui::window, text_cb);
	glfwSetKeyCallback(gui::window, key_cb);
	double next = glfwGetTime();
	int skipped = 1;
	constexpr std::chrono::seconds zero(0);
	constexpr std::chrono::seconds one(1);
	const vec white(1.0f, 1.0f, 1.0f, 1.0f);
	int tps = 64;
#ifdef _DEBUG
#	ifdef LOGIC_TEST
	// Debugging code; define `LOGIC_TEST` in head.hpp to use
	while (1) {
		std::string s;
		std::cin >> s;
		std::transform(s.begin(), s.end(), s.begin(),
			[](char c) { return std::tolower(c); });
		int fails = game::failures;
		if (s == "add")
			photon::photons.push_back(photon(nullptr, 0.0, 0.0, photon::enum_direction::E, 0, 0, nullptr));
		else if (!photon::photons.empty()) {
			int id = 0;
			std::cin >> id;
			photon* p = nullptr;
			std::list<photon>::iterator it = photon::photons.begin();
			for (; it != photon::photons.end(); ++it) {
				if (it->id == id) {
					p = &*it;
					break;
				}
			}
			object spdc = object();
			spdc.type = object::enum_type::SPDC_CRYSTAL;
			object split = object();
			split.type = object::enum_type::SPLITTER;
			object bomb = object();
			bomb.type = object::enum_type::BOMB;
			object wall = object();
			wall.type = object::enum_type::WALL;
			if (s == "spdc")
				interact(p, 0, &spdc, nullptr, 0, tps, &it);
			else if (s == "split")
				interact(p, 0, &split, nullptr, 0, tps, &it);
			else if (s == "bomb")
				interact(p, 0, &bomb, nullptr, 0, tps, &it);
			else if (s == "wall")
				interact(p, 0, &wall, nullptr, 0, tps, &it);
			if (photon::deleting.size() > 0) {
				for (std::list<node>::iterator it = photon::nodes.begin(); it != photon::nodes.end();) {
					node* n = &*it;
					if (std::any_of(photon::deleting.begin(), photon::deleting.end(),
						[n](node* other) { return other == n; }))
						it = photon::nodes.erase(it);
					else
						++it;
				}
				photon::deleting.clear();
			}
		}
		if (game::failures > fails)
			std::cout << "Fail" << std::endl;
		list_nodes(nullptr, 0);
	}
#	endif // LOGIC_TEST
#endif // _DEBUG
	while (!glfwWindowShouldClose(gui::window)) {
		if (gui::quit)
			break;
		double cur = glfwGetTime();
		if (cur - next > 0.5)
			next = cur;
		if (cur >= next) {
			if (gui::menu)
				next += 1.0 / tps;
			else if (!game::hint) {
				double len = 1.0;
				for (int i = 0; i < res::objects.size(); i++)
					if (res::objects.data()[i].moving)
						res::objects.data()[i].tick(tps);
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
					it->pre_tick(tps);
					if (it->_tick < len && it->_tick != 0)
						len = it->_tick;
				}
				int fails = game::failures, level = game::level;
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();) {
					it->tick(tps, len, &it);
					if (game::level > level || game::failures > fails)
						break;
				}
				if (game::level > level) {
					if (game::level >= res::loader::levels.size()) {
						game::level--;
						for (int i = 0; i < 2; i++) {
							glClear(GL_COLOR_BUFFER_BIT);
							for (int layer = 0; layer < 7; layer++)
								for (int i = 0; i < res::objects.size(); i++)
									res::objects.data()[i].render(layer);
							for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
								it->render();
							render_bars();
							render_text(std::string("Time: ") + gui::time(game::time),
								10, 9, 1260, 2, false, 0, white);
							std::string fail_str = std::string("Fails: ") + std::to_string(game::failures);
							render_text(fail_str, 1274 - 18 * (int)fail_str.length(),
								9, 1260, 2, false, 0, white);
							render_text(std::string("Level ") + std::to_string(game::level + 1) + std::string("/") + std::to_string(res::loader::levels.size()),
								10, 689, 1260, 2, false, 0, white);
							if (game::custom) {
								std::string name_str = std::string("Map: ") + game::name;
								render_text(name_str, 1274 - 18 * (int)name_str.length(),
									689, 1260, 2, false, 0, white);
							}
							glfwSwapBuffers(gui::window);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(300));
						for (int i = 1; i <= 10; i++) {
							glEnable(GL_SCISSOR_TEST);
							glScissor(0, 0, 1280, 720);
							glEnable(GL_BLEND);
							glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
							glUniform4f(glGetUniformLocation(res::shaders::rectangle, "colour"), 0.0f, 0.0f, 0.0f, i * 0.1f);
							glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0);
							glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
							glUseProgram(res::shaders::rectangle);
							glBindVertexArray(res::vao);
							glDrawArrays(GL_TRIANGLES, 0, 6);
							glBindVertexArray(0);
							glDisable(GL_BLEND);
							glDisable(GL_SCISSOR_TEST);
							std::this_thread::sleep_for(std::chrono::milliseconds(50));
							glfwSwapBuffers(gui::window);
						}
						game::level++;
						photon::deleting.clear();
						photon::nodes.clear();
						photon::photons.clear();
						object::selected = nullptr;
						object::invalidated.clear();
						object::temp_tex.clear();
						res::objects.clear();
						std::ofstream out(game::save, std::ios::trunc);
						out << (game::hardcore ? 'h' : 'n') << '\n' << game::level << '\n';
						out << game::failures << '\n' << game::time << std::endl;
						out.close();
						game::started = false;
						gui::elements[1]->text = "=SUCCESS=";
						int index = 0;
						for (; index < gui::elements.size() - 1
							&& gui::elements.data()[index]->text.find("Fails:") == std::string::npos; index++);
						gui::elements.data()[index]->text = std::string("Fails: ") + std::to_string(game::failures);
						gui::elements.data()[index]->visible = true;
						gui::elements.data()[index + 2]->text = std::string("Time: ") + gui::time(game::time);
						gui::elements.data()[index + 2]->visible = true;
						gui::menu = true;
					}
					else {
						std::this_thread::sleep_for(one);
						glClear(GL_COLOR_BUFFER_BIT);
						glfwSwapBuffers(gui::window);
					}
					std::this_thread::sleep_for(one);
					res::loader::load_level(game::level, true);
					game::frame = -1;
				}
				else if (game::failures == fails) {
					bool fail = true;
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
						if (it->direction != photon::enum_direction::NONE) {
							fail = false;
							break;
						}
					}
					if (fail || photon::photons.empty()) {
						game::failures++;
						std::this_thread::sleep_for(one);
						res::loader::load_level(game::level, true);
						game::frame = -1;
					}
					else {
						for (std::unordered_set<node*>::iterator it = photon::deleting.begin(); it != photon::deleting.end(); ++it) {
							(*it)->clear();
							photon::nodes.remove_if([it](const node& other) { return &other == *it; });
						}
						if (photon::deleting.size() > 0) {
							for (std::list<node>::iterator it = photon::nodes.begin(); it != photon::nodes.end();) {
								node* n = &*it;
								if (std::any_of(photon::deleting.begin(), photon::deleting.end(),
									[n](node* other) { return other == n; }))
									it = photon::nodes.erase(it);
								else
									++it;
							}
							photon::deleting.clear();
						}
						next += len / tps;
						game::time += len / tps;
					}
				}
				else if (game::hardcore) {
					for (int i = 0; i < 2; i++) {
						glClear(GL_COLOR_BUFFER_BIT);
						for (int layer = 0; layer < 7; layer++)
							for (int i = 0; i < res::objects.size(); i++)
								res::objects.data()[i].render(layer);
						for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
							it->render();
						render_bars();
						render_text(std::string("Time: ") + gui::time(game::time),
							10, 9, 1260, 2, false, 0, white);
						std::string fail_str = std::string("Fails: ") + std::to_string(game::failures);
						render_text(fail_str, 1274 - 18 * (int)fail_str.length(),
							9, 1260, 2, false, 0, white);
						render_text(std::string("Level ") + std::to_string(game::level + 1) + std::string("/") + std::to_string(res::loader::levels.size()),
							10, 689, 1260, 2, false, 0, white);
						if (game::custom) {
							std::string name_str = std::string("Map: ") + game::name;
							render_text(name_str, 1274 - 18 * (int)name_str.length(),
								689, 1260, 2, false, 0, white);
						}
						glfwSwapBuffers(gui::window);
					}
					for (int i = 3; i <= 10; i++) {
						object::reason->render(-2, i);
						std::this_thread::sleep_for(std::chrono::milliseconds(30));
						glfwSwapBuffers(gui::window);
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					for (int i = 5; i <= 10; i++) {
						glEnable(GL_SCISSOR_TEST);
						glScissor(0, 0, 1280, 720);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						glUniform4f(glGetUniformLocation(res::shaders::rectangle, "colour"), 1.0f, 1.0f, 1.0f, i * 0.1f);
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0);
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
						glUseProgram(res::shaders::rectangle);
						glBindVertexArray(res::vao);
						glDrawArrays(GL_TRIANGLES, 0, 6);
						glBindVertexArray(0);
						glDisable(GL_BLEND);
						glDisable(GL_SCISSOR_TEST);
						std::this_thread::sleep_for(std::chrono::milliseconds(30));
						glfwSwapBuffers(gui::window);
					}
					std::this_thread::sleep_for(std::chrono::seconds(3));
					photon::deleting.clear();
					photon::nodes.clear();
					photon::photons.clear();
					object::selected = nullptr;
					object::invalidated.clear();
					object::temp_tex.clear();
					res::objects.clear();
					game::failures = -1;
					std::ofstream out(game::save, std::ios::trunc);
					out << (game::hardcore ? 'h' : 'n') << '\n' << game::level << '\n';
					out << game::failures << '\n' << game::time << std::endl;
					out.close();
					game::started = false;
					gui::elements[1]->text = "GAME OVER";
					int index = 0;
					for (; index < gui::elements.size() - 2
						&& gui::elements.data()[index]->text.find("Level:") == std::string::npos; index++);
					gui::elements.data()[index]->text = std::string("Level: ") + std::to_string(game::level);
					gui::elements.data()[index]->visible = true;
					gui::elements.data()[index + 1]->text = std::string("Time: ") + gui::time(game::time);
					gui::elements.data()[index + 1]->visible = true;
					gui::menu = true;
				}
				else {
					for (int i = 0; i < 2; i++) {
						glClear(GL_COLOR_BUFFER_BIT);
						for (int layer = 0; layer < 7; layer++)
							for (int i = 0; i < res::objects.size(); i++)
								res::objects.data()[i].render(layer);
						for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
							it->render();
						render_bars();
						render_text(std::string("Time: ") + gui::time(game::time),
							10, 9, 1260, 2, false, 0, white);
						std::string fail_str = std::string("Fails: ") + std::to_string(game::failures);
						render_text(fail_str, 1274 - 18 * (int)fail_str.length(),
							9, 1260, 2, false, 0, white);
						render_text(std::string("Level ") + std::to_string(game::level + 1) + std::string("/") + std::to_string(res::loader::levels.size()),
							10, 689, 1260, 2, false, 0, white);
						if (game::custom) {
							std::string name_str = std::string("Map: ") + game::name;
							render_text(name_str, 1274 - 18 * (int)name_str.length(),
								689, 1260, 2, false, 0, white);
						}
						glfwSwapBuffers(gui::window);
					}
					for (int i = 1; i <= 5; i++) {
						object::reason->render(-2, i);
						std::this_thread::sleep_for(std::chrono::milliseconds(30));
						glfwSwapBuffers(gui::window);
					}
					std::this_thread::sleep_for(one);
					res::loader::load_level(game::level, true);
					game::frame = -1;
				}
				object::reason = nullptr;
			}
			if (cur < next || skipped > 5) {
				game::frame++;
				if (gui::menu) {
					glClear(GL_COLOR_BUFFER_BIT);
					for (int i = 0; i < gui::elements.size(); i++)
						gui::elements.data()[i]->render();
				}
				else if (game::hint && !res::loader::levels.data()[game::level].hint.empty()) {
					glClear(GL_COLOR_BUFFER_BIT);
					render_text(res::loader::levels.data()[game::level].hint, 50, 50, 1180, 2, false, 0, white);
					render_text("Press any key to dismiss", 50, 648, 1180, 2, false, 0, white);
				}
				else if ((game::frame % 10 - game::frame % 2) && !object::invalidate_all) {
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
						glEnable(GL_SCISSOR_TEST);
						glScissor(
							(GLint)std::round(it->_x - 480.0 / tps)* PX_SIZE, 40 + (GLint)std::round(it->_y - 480.0 / tps)* PX_SIZE,
							(GLsizei)(PX_SIZE * 960.0 / tps), (GLsizei)(PX_SIZE * 960.0 / tps));
						glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
							1, res::loader::background().colour.ptr());
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"),
							res::loader::background().noise);
						glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
						glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
							(GLfloat)res::objects.data()[0].offset,
							(GLfloat)res::objects.data()[0].offset);
						glUseProgram(res::shaders::rectangle);
						glBindVertexArray(res::vao);
						glDrawArrays(GL_TRIANGLES, 0, 6);
						glBindVertexArray(0);
						glDisable(GL_SCISSOR_TEST);
					}
					if (object::selected) {
						if (object::selected->linked) {
							for (int i = 0; i < object::selected->linked->members.size(); i++)
								object::selected->linked->members.data()[i]->border(true);
						}
						else
							object::selected->border(true);
					}
					for (int i = 1; i < res::objects.size(); i++) {
						object& obj = res::objects.data()[i];
						static constexpr object::enum_type translucent[] = {
							object::enum_type::DIAGONAL_MIRROR,
							object::enum_type::GLASS_BLOCK, object::enum_type::FIXED_BLOCK,
							object::enum_type::MOVING_BLOCK, object::enum_type::PRISM,
							object::enum_type::SPDC_CRYSTAL, object::enum_type::MOVING_CRYSTAL,
							object::enum_type::SPLITTER
						};
						static const object::enum_type* end = translucent + sizeof(translucent) / sizeof(object::enum_type);
						if (std::find(translucent, end, obj.type) != end || (obj.toggle && (obj.type == object::enum_type::DOOR || obj.type == object::enum_type::MIRROR_DOOR)))
							object::invalidated.insert(&obj);
						else if ((obj.type == object::enum_type::DOOR || obj.type == object::enum_type::MIRROR_DOOR) && obj.toggle)
							object::invalidated.insert(&obj);
						else if (std::any_of(photon::photons.begin(), photon::photons.end(), [obj, tps](photon p) {
							return obj.type != object::enum_type::NONE
								&& obj_dist(p._x, p._y, obj) <= 960.0 / tps;
							}))
							object::invalidated.insert(&obj);
						else if (object::previous && object::previous->linked
								&& std::any_of(object::previous->linked->members.begin(), object::previous->linked->members.end(),
									[obj](object* o) { return o == &obj; }))
							object::invalidated.insert(&obj);
						else if (object::selected && object::selected->linked
								&& std::any_of(object::selected->linked->members.begin(), object::selected->linked->members.end(),
									[obj](object* o) {
										object temp = *o;
										temp.x -= 4;
										temp.y -= 4;
										temp.width += 8;
										temp.height += 8;
										return object::overlapping(temp, obj);
									}))
							object::invalidated.insert(&obj);
						else if (object::selected && !object::selected->linked) {
							object temp = *object::selected;
							temp.x -= 4;
							temp.y -= 4;
							temp.width += 8;
							temp.height += 8;
							if (object::overlapping(temp, obj))
								object::invalidated.insert(&obj);
						}
					}
					if (object::previous && !object::previous->linked)
						object::invalidated.insert(object::previous);
					for (size_t count = -1; count != object::invalidated.size(); count = object::invalidated.size()) {
						for (int i = 1; i < res::objects.size(); i++) {
							object& obj = res::objects.data()[i];
							if (std::any_of(object::invalidated.begin(), object::invalidated.end(), [obj, tps](object* other) {
								return obj.type != object::enum_type::NONE && object::overlapping(obj, *other);		}))
								object::invalidated.insert(&obj);
						}
					}
					for (int layer = -1; layer < 7; layer++)
						for (std::unordered_set<object*>::iterator it = object::invalidated.begin();
							it != object::invalidated.end(); ++it)
							(*it)->render(layer);
					if (game::frame % 2)
						object::invalidated.clear();
				}
				else {
					if (object::invalidate_all)
						game::frame = 0;
					object::invalidated.clear();
					object::invalidate_all = false;
					glClear(GL_COLOR_BUFFER_BIT);
					for (int layer = 0; layer < 7; layer++)
						for (int i = 0; i < res::objects.size(); i++)
							res::objects.data()[i].render(layer);
				}
				if (!gui::menu && !game::hint) {
					if (object::selected) {
						if (object::selected->linked) {
							for (int i = 0; i < object::selected->linked->members.size(); i++)
								object::selected->linked->members.data()[i]->border();
						}
						else
							object::selected->border();
					}
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
						it->render();
					render_bars();
					render_text(std::string("Time: ") + gui::time(game::time),
						10, 9, 1260, 2, false, 0, white);
					std::string fail_str = std::string("Fails: ") + std::to_string(game::failures);
					render_text(fail_str, 1274 - 18 * (int)fail_str.length(),
						9, 1260, 2, false, 0, white);
					render_text(std::string("Level ") + std::to_string(game::level + 1) + std::string("/") + std::to_string(res::loader::levels.size()),
						10, 689, 1260, 2, false, 0, white);
					if (game::custom) {
						std::string name_str = std::string("Map: ") + game::name;
						render_text(name_str, 1274 - 18 * (int)name_str.length(),
							689, 1260, 2, false, 0, white);
					}
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
	if (!game::save.empty()) {
		std::ofstream out(game::save, std::ios::trunc);
		out << (game::hardcore ? 'h' : 'n') << '\n' << game::level << '\n';
		out << game::failures << '\n' << game::time << std::endl;
		out.close();
	}
	std::ofstream cfg("./settings", std::ios::trunc);
	cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
	cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
	cfg << keybinds::hint << '\n' << game::save << std::endl;
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
				else if (game::started) {
					glClear(GL_COLOR_BUFFER_BIT);
					game::frame = -1;
					gui::menu = false;
				}
			}
		}
		else
			for (int i = 0; i < gui::elements.size(); i++)
				gui::elements.data()[i]->handler(gui::enum_event::KEY, key, action, mods);
		return;
	}
	if (action != GLFW_PRESS)
		return;
	if (game::hint) {
		game::hint = false;
		return;
	}
	int step = 0;
	if (object::selected) {
		switch (object::selected->type) {
		case object::enum_type::MIRROR:     	step = 1;	break;
		case object::enum_type::GLASS_BLOCK:	step = 2;	break;
		default: break;
		}
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
			object::invalidate_all = true;
		}
	}
	else if (key == keybinds::cw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 0 ? 16 - step : dir - step);
			object::invalidate_all = true;
		}
	}
	else if (key == keybinds::perp) {
		if (object::selected) {
			int dir = (int)object::selected->orientation + 4;
			if (dir > 15)
				dir -= 16;
			object::selected->orientation = (object::enum_orientation)dir;
			object::invalidate_all = true;
		}
	}
	else if (key == keybinds::toggle) {
		if (object::selected && !object::selected->moving) {
			object::selected->toggle = !object::selected->toggle;
			if (object::selected->linked) {
				for (int i = 0; i < object::selected->linked->members.size(); i++) {
					object* obj = object::selected->linked->members.data()[i];
					if (!obj->moving) {
						obj->toggle = object::selected->toggle;
						if (obj->type == object::enum_type::MOVING_WALL
							|| obj->type == object::enum_type::MOVING_BLOCK
							|| obj->type == object::enum_type::MOVING_CRYSTAL)
							obj->moving = true;
						else
							object::invalidate_all = true;
					}
				}
			}
			else {
				if (object::selected->type == object::enum_type::MOVING_WALL
					|| object::selected->type == object::enum_type::MOVING_BLOCK
					|| object::selected->type == object::enum_type::MOVING_CRYSTAL)
					object::selected->moving = true;
				else
					object::invalidate_all = true;
			}
		}
	}
	else if (key == keybinds::hint)
		game::hint = true;
	else if (key == GLFW_KEY_ESCAPE) {
		std::ofstream out(game::save, std::ios::trunc);
		out << (game::hardcore ? 'h' : 'n') << '\n' << game::level << '\n';
		out << game::failures << '\n' << game::time << std::endl;
		out.close();
		gui::menu = true;
	}
}

namespace keybinds {
	int up = GLFW_KEY_W;
	int left = GLFW_KEY_A;
	int down = GLFW_KEY_S;
	int right = GLFW_KEY_D;
	int ccw = GLFW_KEY_LEFT_BRACKET;
	int cw = GLFW_KEY_RIGHT_BRACKET;
	int perp = GLFW_KEY_BACKSLASH;
	int toggle = GLFW_KEY_SLASH;
	int hint = GLFW_KEY_H;
}

#ifdef LOGIC_TEST
static bool list_nodes(node* root, int depth) {
	std::vector<photon*> items;
	std::vector<node*> children;
	if (root) {
		items = root->items;
		children = root->children;
	}
	else {
		for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it)
			if (it->parent == nullptr)
				items.push_back(&*it);
		for (std::list<node>::iterator it = photon::nodes.begin(); it != photon::nodes.end(); ++it)
			if (it->parent == nullptr)
				children.push_back(&*it);
	}
	for (int i = 0; i < depth; i++)
		std::cout << "  ";
	if (root)
		std::cout << (root->type == node::enum_node::SPDC ? "SPDC" : "SUPERPOS") << std::endl;
	else
		std::cout << "ROOT" << std::endl;
	for (int i = 0; i < items.size(); i++) {
		for (int i = 0; i <= depth; i++)
			std::cout << "  ";
		std::cout << items.data()[i]->id << " " << items.data()[i]->split;
		if (items.data()[i]->direction == photon::enum_direction::NONE)
			std::cout << " !";
		std::cout << std::endl;
	}
	for (int i = 0; i < children.size(); i++)
		list_nodes(children.data()[i], depth + 1);
	return false;
}
#endif // LOGIC_TEST
