// If you want to use the editor, exclude the original
// main.cpp file from compilation (select it and go
// into Properties) and compile with this one instead

// (This file is excluded from compilation by default)
// (Don't try compiling with both versions of main, it won't work)

#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../head.hpp"

#define TEX(x) &res::loader::textures[x]
#define HBX(x) &res::loader::hitboxes[x]

#define TEX_PHOTON  	TEX(0)

#define TEX_WALL    	TEX(1)
#define TEX_DOOR    	TEX(2)
#define TEX_MOV_WALL	TEX(4)
#define TEX_ROT_MIRROR	TEX(5)
#define TEX_DMIRROR 	TEX(13)
#define TEX_MIRRORBLOCK	TEX(15)
#define TEX_MIRRORDOOR	TEX(16)
#define TEX_ROT_BLOCK	TEX(18)
#define TEX_FBLOCK  	TEX(22)
#define TEX_MOV_BLOCK	TEX(23)
#define TEX_PRISM   	TEX(24)
#define TEX_SPDC    	TEX(25)
#define TEX_MOV_SPDC	TEX(26)
#define TEX_SPLITTER	TEX(27)
#define TEX_BOMB    	TEX(29)
#define TEX_SENSOR  	TEX(31)

#define HBX_WALL    	NULL
#define HBX_DOOR    	NULL
#define HBX_MOV_WALL	NULL
#define HBX_ROT_MIRROR	HBX(0)
#define HBX_DMIRROR 	HBX(8)
#define HBX_MIRRORBLOCK	NULL
#define HBX_MIRRORDOOR	NULL
#define HBX_ROT_BLOCK	HBX(10)
#define HBX_FBLOCK  	NULL
#define HBX_MOV_BLOCK	NULL
#define HBX_PRISM   	HBX(14)
#define HBX_SPDC    	NULL
#define HBX_MOV_SPDC	NULL
#define HBX_SPLITTER	HBX(15)
#define HBX_BOMB    	NULL
#define HBX_SENSOR  	NULL

namespace fs = std::filesystem;

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);

static bool command = false;

int main(void) {
	srand((unsigned)time(NULL));
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
	if (fs::is_regular_file(fs::path("./settings"))) {
		std::ifstream in("./settings");
		in >> keybinds::up >> keybinds::left >> keybinds::down >> keybinds::right;
		in >> keybinds::ccw >> keybinds::cw >> keybinds::perp >> keybinds::toggle;
		in.close();
	}
	res::load_vao();
	res::loader::load_tex();
	res::loader::load_hbx();
	res::loader::load_default();
	res::shaders::load();
	glfwSetKeyCallback(window, key_cb);
	double next = glfwGetTime();
	int skipped = 1;
	constexpr std::chrono::seconds zero(0);
	int tps = 64;
	std::cout << "Type `help` for a list of commands." << std::endl;
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
	while (!glfwWindowShouldClose(window)) {
		command = false;
		bool preview = false;
		while (1) {
			std::string cmd;
			std::cout << "> ";
			std::cin >> cmd;
			std::transform(cmd.begin(), cmd.end(), cmd.begin(),
				[](char c) { return std::tolower(c); });
			std::vector<object>* current = &res::loader::levels.data()[gamestate::level].objects;
			if (cmd == "help") {
				std::cout << "All parameters in [square brackets] are integers. All parameters in (parentheses) are strings.\n";
				std::cout << "List of commands:\n";
				std::cout << "help          - Displays this list.\n";
				std::cout << "cheatsheet    - Displays the numerical values of directions, orientations, and object types.\n";
				std::cout << "load (path)   - Loads a level from a file for editing.\n";
				std::cout << "export (path) - Exports the level to a file. You can export the default level to see what the file is like.\n";
				std::cout << "save          - Like `export`, but does not warn before overwriting. Must have used `load`/`export` first.\n";
				std::cout << "levels        - Lists all levels as well as their object counts.\n";
				std::cout << "level [n]     - Selects the [n]th level. If this level does not exist, it will be created.\n";
				std::cout << "remove [n]    - Removes the [n]th level.\n";
				std::cout << "text (str)    - Sets the text to be displayed before loading the selected level to (str).\n";
				std::cout << "rmtext        - Removes the text displayed before the selected level.\n";
				std::cout << "list          - Lists all objects in the selected level.\n";
				std::cout << "add [type] [x] [y] [o]\n";
				std::cout << "              - Places an object of type [type] at ([x], [y]) with orientation [o]. See `cheatsheet`.\n";
				std::cout << "move [id] [x] [y] [o]\n";
				std::cout << "              - Moves the object specified by [id] to coordinates ([x], [y]) and orientation [o].\n";
				std::cout << "inspect [id]  - Diplays information on the object specified by [id]. Use `list` for a list of IDs.\n";
				std::cout << "delete [id]   - Removes the object specified by [id]. Use `list` for a list of IDs.\n";
				std::cout << "clear         - Removes all objects in the selected level.\n";
				std::cout << "spawn [x] [y] [dir]\n";
				std::cout << "              - Sets the spawn position of the photon to([x], [y]) with initial direction[dir]. See `cheatsheet`.\n";
				std::cout << "preview       - Renders the selected level.\n";
				std::cout << "play          - Plays the selected level.\n";
				std::cout << "credits       - Displays credits.\n";
				std::cout << "quit          - Exits the game.\n";
				std::cout << "exit          - Exits the game." << std::endl;
			}
			else if (cmd == "cheatsheet") {
				std::cout << "Directions     | Orientationss | Types\n";
				std::cout << "---------------+---------------+--------------------\n";
				std::cout << "E         = 0  | N    = 0      | WALL            = 0\n";
				std::cout << "E_ATAN1_3 = 1  | NNW  = 1      | DOOR            = 1\n";
				std::cout << "E_ATAN1_2 = 2  | NW   = 2      | MOVING_WALL     = 2\n";
				std::cout << "NE        = 3  | NWW  = 3      | MIRROR          = 3\n";
				std::cout << "E_ATAN2   = 4  | W    = 4      | DIAGONAL_MIRROR = 4\n";
				std::cout << "E_ATAN3   = 5  | SWW  = 5      | MIRROR_BLOCK    = 5\n";
				std::cout << "N         = 6  | SW   = 6      | MIRROR_DOOR     = 6\n";
				std::cout << "N_ATAN1_3 = 7  | SSW  = 7      | GLASS_BLOCK     = 7\n";
				std::cout << "N_ATAN1_2 = 8  | S    = 8      | FIXED_BLOCK     = 8\n";
				std::cout << "NW        = 9  | SSE  = 9      | MOVING_BLOCK    = 9\n";
				std::cout << "N_ATAN2   = 10 | SE   = 10     | PRISM           = 10\n";
				std::cout << "N_ATAN3   = 11 | SEE  = 11     | SPDC_CRYSTAL    = 11\n";
				std::cout << "W         = 12 | E    = 12     | MOVING_CRYSTAL  = 12\n";
				std::cout << "W_ATAN1_3 = 13 | NEE  = 13     | SPLITTER        = 13\n";
				std::cout << "W_ATAN1_2 = 14 | NE   = 14     | BOMB            = 14\n";
				std::cout << "SW        = 15 | NNE  = 15     | SENSOR          = 15\n";
				std::cout << "W_ATAN2   = 16 | NONE = 16     | NONE            = 16\n";
				std::cout << "W_ATAN3   = 17 |               |\n";
				std::cout << "S         = 18 |               |\n";
				std::cout << "S_ATAN1_3 = 19 |               |\n";
				std::cout << "S_ATAN1_2 = 20 |               |\n";
				std::cout << "SW        = 21 |               |\n";
				std::cout << "S_ATAN2   = 22 |               |\n";
				std::cout << "S_ATAN3   = 23 |               |\n";
				std::cout << "NONE      = 24 |               |" << std::endl;
			}
			else if (cmd == "load") {
				std::string path;
				std::getline(std::cin, path);
				path = path.substr(1);
				if (fs::is_regular_file(path)) {
					if (!res::loader::load_from_file(path)) {
						std::cout << "Failed to read level file" << std::endl;
						res::loader::load_default();
					}
					gamestate::save = path;
				}
				else
					std::cout << "File not found" << std::endl;
			}
			else if (cmd == "export") {
				std::string path;
				std::getline(std::cin, path);
				path = path.substr(1);
				if (fs::is_regular_file(path)) {
					char c;
					do {
						std::cout << "The file at " << path << " already exists. Would you like to overwrite it? (Y/N) ";
						std::string s;
						std::cin >> s;
						c = std::toupper(s.front());
					} while (c != 'Y' && c != 'N');
					if (c == 'N')
						continue;
				}
				gamestate::save = path;
				std::ofstream out(path, std::ios::trunc);
				for (int i = 0; i < res::loader::levels.size(); i++) {
					out << " " << res::loader::levels.data()[i].hint << "\n";
					const object& p = res::loader::levels.data()[i].objects.data()[0];
					out << p.x << " " << p.y << " " << p.data << "\n";
					for (int j = 2; j < res::loader::levels.data()[i].objects.size(); j++) {
						const object& obj = res::loader::levels.data()[i].objects.data()[j];
						out << (int)obj.type << " " << obj.x << " " << obj.y << " " << (int)obj.orientation;
						if (obj.type == object::enum_type::MOVING_WALL
							|| obj.type == object::enum_type::MOVING_BLOCK
							|| obj.type == object::enum_type::MOVING_CRYSTAL) {
							out << " " << obj.x2 << " " << obj.y2 << " " << obj.data;
						}
						out << "\n";
					}
				}
				std::flush(out);
				if (out.fail())
					std::cout << "Something went wrong while writing to the file" << std::endl;
				out.close();
			}
			else if (cmd == "save") {
				if (gamestate::save.empty()) {
					std::cout << "No level file loaded. Try `export`." << std::endl;
					continue;
				}
				std::ofstream out(gamestate::save, std::ios::trunc);
				for (int i = 0; i < res::loader::levels.size(); i++) {
					out << " " << res::loader::levels.data()[i].hint << "\n";
					const object& p = res::loader::levels.data()[i].objects.data()[0];
					out << p.x << " " << p.y << " " << p.data << "\n";
					for (int j = 2; j < res::loader::levels.data()[i].objects.size(); j++) {
						const object& obj = res::loader::levels.data()[i].objects.data()[j];
						out << (int)obj.type << " " << obj.x << " " << obj.y << " " << (int)obj.orientation;
						if (obj.type == object::enum_type::MOVING_WALL
							|| obj.type == object::enum_type::MOVING_BLOCK
							|| obj.type == object::enum_type::MOVING_CRYSTAL) {
							out << " " << obj.x2 << " " << obj.y2 << " " << obj.data;
						}
						out << "\n";
					}
				}
				std::flush(out);
				if (out.fail())
					std::cout << "Something went wrong while writing to the file" << std::endl;
				out.close();
			}
			else if (cmd == "levels")
				for (int i = 0; i < res::loader::levels.size(); i++)
					std::cout << "Level " << i + 1 << " - "
						<< res::loader::levels.data()[i].objects.size() - 2 << " objects\n\""
						<< res::loader::levels.data()[i].hint << "\"" << std::endl;
			else if (cmd == "level") {
				std::string _n;
				std::cin >> _n;
				int n;
				try {
					n = std::stoi(_n);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (n < 1) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if ((size_t)(n - 1) == res::loader::levels.size()) {
					res::loader::levels.push_back(res::loader::level());
					res::loader::levels.back().objects.push_back(object(0.0, 0.0, 1, 1,
						object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL,
						(int)photon::enum_direction::E));
					res::loader::levels.back().objects.push_back(object(0.0, 0.0, 640, 360,
						object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0));
				}
				if ((size_t)(n - 1) <= res::loader::levels.size())
					gamestate::level = n - 1;
				else
					std::cout << "Cannot create level " << n << " as level "
						<< res::loader::levels.size() + 1 << " does not exist yet" << std::endl;
			}
			else if (cmd == "remove") {
				std::string _n;
				std::cin >> _n;
				int n;
				try {
					n = std::stoi(_n);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (n < 1) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (res::loader::levels.size() <= 1) {
					std::cout << "Can't remove only remaining level" << std::endl;
					continue;
				}
				char c;
				do {
					std::cout << "Are you sure you want to remove level " << n
						<< " (" << res::loader::levels.data()[n - 1].objects.size() - 2 << " objects)? (Y/N) ";
					std::string s;
					std::cin >> s;
					c = std::toupper(s.front());
				} while (c != 'Y' && c != 'N');
				if (c == 'Y')
					res::loader::levels.erase(res::loader::levels.begin() + (n - 1));
			}
			else if (cmd == "text") {
				std::string text;
				std::getline(std::cin, text);
				res::loader::levels.data()[gamestate::level].hint = text.substr(1);
			}
			else if (cmd == "rmtext")
				res::loader::levels.data()[gamestate::level].hint.clear();
			else if (cmd == "list") {
				for (int i = 2; i < current->size(); i++) {
					std::cout << "id=" << i - 2 << " | type=" << (int)current->data()[i].type
						<< ", x=" << current->data()[i].x << ", y=" << current->data()[i].y
						<< ", orientation=" << (int)current->data()[i].orientation
						<< ", otherx=" << current->data()[i].x2 << ", othery=" << current->data()[i].y2
						<< ", number=" << current->data()[i].data << std::endl;
				}
			}
			else if (cmd == "add") {
				std::string _type, _x, _y, _o;
				std::cin >> _type >> _x >> _y >> _o;
				int x, y, o, type;
				try {
					type = std::stoi(_type);
					x = std::stoi(_x);
					y = std::stoi(_y);
					o = std::stoi(_o);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (o < 0 || o > (int)object::enum_orientation::NONE
					|| type < 0 || type > (int)object::enum_type::NONE) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				object::enum_type t = (object::enum_type)type;
				current->push_back(object(x, y, 20, 20,
					(object::enum_orientation)o, t,
					res::loader::get_tex(t), res::loader::get_hbx(t), 0));
				if (t == object::enum_type::MOVING_WALL
					|| t == object::enum_type::MOVING_BLOCK
					|| t == object::enum_type::MOVING_CRYSTAL) {
					int x2, y2, link;
					while (1) {
						try {
							std::cout << "This object can move. Enter the other point ([x2] [y2]) that this object can move to: ";
							std::string _x2, _y2;
							std::cin >> _x2 >> _y2;
							x2 = std::stoi(_x2);
							y2 = std::stoi(_y2);
							break;
						}
						catch (...) {
							std::cout << "Invalid input" << std::endl;
						}
					}
					while (1) {
						try {
							std::cout << "Enter a number. All moving objects with the same number (except 0) will move in tandem. ";
							std::string _link;
							std::cin >> _link;
							link = std::stoi(_link);
							if (link < 0)
								std::cout << "Invalid input" << std::endl;
							else
								break;
						}
						catch (...) {
							std::cout << "Invalid input" << std::endl;
						}
					}
					current->back().x2 = x2;
					current->back().y2 = y2;
					current->back().data = link;
				}
			}
			else if (cmd == "move") {
				std::string _id, _x, _y, _o;
				std::cin >> _id >> _x >> _y >> _o;
				int id, x, y, o;
				try {
					id = std::stoi(_id);
					x = std::stoi(_x);
					y = std::stoi(_y);
					o = std::stoi(_o);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()
					|| o < 0 || o > (int)object::enum_orientation::NONE) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				current->data()[id].x = x;
				current->data()[id]._x = x;
				current->data()[id].y = y;
				current->data()[id]._y = y;
				current->data()[id].orientation = (object::enum_orientation)o;
			}
			else if (cmd == "inspect") {
				std::string _id;
				std::cin >> _id;
				int id;
				try {
					id = std::stoi(_id);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				std::cout << "type=" << (int)current->data()[id].type
					<< ", x=" << current->data()[id].x << ", y=" << current->data()[id].y
					<< ", orientation=" << (int)current->data()[id].orientation
					<< ", otherx=" << current->data()[id].x2 << ", othery=" << current->data()[id].y2
					<< ", number=" << current->data()[id].data << std::endl;
			}
			else if (cmd == "delete") {
				std::string _id;
				std::cin >> _id;
				int id;
				try {
					id = std::stoi(_id);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (id < 0 || id >= current->size() - 2) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				current->erase(current->begin() + (id + 2));
			}
			else if (cmd == "clear") {
				char c;
				do {
					std::cout << "Are you sure you want to clear the current level? (Y/N) ";
					std::string s;
					std::cin >> s;
					c = std::toupper(s.front());
				} while (c != 'Y' && c != 'N');
				if (c == 'Y') {
					current->clear();
					current->push_back(object(0.0, 0.0, 1, 1,
						object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL,
						(int)photon::enum_direction::E));
					current->push_back(object(0.0, 0.0, 640, 360,
						object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0));
				}
			}
			else if (cmd == "spawn") {
				std::string _x, _y, _dir;
				std::cin >> _x >> _y >> _dir;
				int x, y, dir;
				try {
					x = std::stoi(_x);
					y = std::stoi(_y);
					dir = std::stoi(_dir);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				if (dir >= (int)photon::enum_direction::NONE) {
					std::cout << "Invalid input" << std::endl;
					continue;
				}
				current->data()[0].x = x;
				current->data()[0].y = y;
				current->data()[0].data = dir;
			}
			else if (cmd == "preview") {
				res::loader::load_level(gamestate::level, false);
				preview = true;
				glfwFocusWindow(window);
				break;
			}
			else if (cmd == "play") {
				std::cout << "Press ESC to return to the console." << std::endl;
				res::loader::load_level(gamestate::level, false);
				glfwFocusWindow(window);
				break;
			}
			else if (cmd == "credits") {
				std::cout << "This game uses the following libraries:\n";
				std::cout << "GLFW: Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla Löwy; licensed under the zlib License\n";
				std::cout << "Glad: Copyright (c) 2013-2022 David Herberth; licensed under the MIT License" << std::endl;
			}
			else if (cmd == "quit" || cmd == "exit") {
				glfwDestroyWindow(window);
				glfwTerminate();
				exit(EXIT_SUCCESS);
			}
			else
				std::cout << "Unrecognized command" << std::endl;
		}
		while (!glfwWindowShouldClose(window)) {
			if (preview) {
				for (int i = 0; i < 2; i++) {
					glClear(GL_COLOR_BUFFER_BIT);
					for (int layer = 0; layer < 7; layer++)
						for (int i = 0; i < res::objects.size(); i++)
							res::objects.data()[i].render(layer);
					glfwSwapBuffers(window);
				}
				break;
			}
			if (command)
				break;
			double cur = glfwGetTime();
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
				int fails = gamestate::failures, level = gamestate::level;
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();)
					it->tick(tps, len, &it);
				if (gamestate::level > level)
					break;
				else if (gamestate::failures == fails) {
					bool fail = true;
					for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
						if (it->direction != photon::enum_direction::NONE) {
							fail = false;
							break;
						}
					}
					if (fail || photon::photons.empty())
						res::loader::load_level(level, false);
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
					}
				}
				else
					res::loader::load_level(level, false);
				if (cur < next || skipped > 5) {
					glClear(GL_COLOR_BUFFER_BIT);
					for (int layer = 0; layer < 7; layer++)
						for (int i = 0; i < res::objects.size(); i++)
							res::objects.data()[i].render(layer);
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
					glfwSwapBuffers(window);
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
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return EXIT_SUCCESS;
}

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS)
		return;
	int step = 0;
	if (object::selected) {
		switch (object::selected->type) {
		case object::enum_type::MIRROR:			step = 1;	break;
		case object::enum_type::GLASS_BLOCK:	step = 2;	break;
		default:											break;
		}
	}
	if (key == keybinds::up || key == keybinds::left
		|| key == keybinds::down || key == keybinds::right)
		select(key);
	else if (key == keybinds::ccw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
			object::selected->orientation = (object::enum_orientation)(dir == 16 - step ? 0 : dir + step);
		}
	}
	else if (key == keybinds::cw) {
		if (object::selected) {
			int dir = (int)object::selected->orientation;
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
			if (object::selected->linked) {
				for (int i = 0; i < object::selected->linked->members.size(); i++) {
					object* obj = object::selected->linked->members.data()[i];
					obj->toggle = object::selected->toggle;
					if (obj->type == object::enum_type::MOVING_WALL
						|| obj->type == object::enum_type::MOVING_BLOCK
						|| obj->type == object::enum_type::MOVING_CRYSTAL)
						obj->moving = true;
				}
			}
			else {
				if (object::selected->type == object::enum_type::MOVING_WALL
					|| object::selected->type == object::enum_type::MOVING_BLOCK
					|| object::selected->type == object::enum_type::MOVING_CRYSTAL)
					object::selected->moving = true;
			}
		}
	}
	else if (key == GLFW_KEY_ESCAPE)
		command = true;
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
}
