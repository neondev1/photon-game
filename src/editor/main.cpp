// If you want to use the editor, compile the project using the EditorRelease configuration

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>
#include <thread>

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

#define HBX_WALL    	nullptr
#define HBX_DOOR    	nullptr
#define HBX_MOV_WALL	nullptr
#define HBX_ROT_MIRROR	HBX(0)
#define HBX_DMIRROR 	HBX(8)
#define HBX_MIRRORBLOCK	nullptr
#define HBX_MIRRORDOOR	nullptr
#define HBX_ROT_BLOCK	HBX(10)
#define HBX_FBLOCK  	nullptr
#define HBX_MOV_BLOCK	nullptr
#define HBX_PRISM   	HBX(14)
#define HBX_SPDC    	nullptr
#define HBX_MOV_SPDC	nullptr
#define HBX_SPLITTER	HBX(15)
#define HBX_BOMB    	nullptr
#define HBX_SENSOR  	nullptr

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

void key_cb(GLFWwindow* window, int key, int scancode, int action, int mods);
bool save_to_file(std::string path);

static bool command = false;
long long game::frame = -1;

int main(void) {
#ifdef _WIN32
	DisableProcessWindowsGhosting();
#endif // _WIN32
	if (!glfwInit()) {
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to initialize GLFW", "", MB_OK | MB_ICONERROR);
#else
		std::cerr << "Failed to initialize GLFW" << std::endl;
#endif // _WIN32
		return EXIT_FAILURE;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Photon", nullptr, nullptr);
	if (window == nullptr) {
		glfwTerminate();
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to create window", "", MB_OK | MB_ICONERROR);
#else
		std::cerr << "Failed to create window" << std::endl;
#endif // _WIN32
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		glfwDestroyWindow(window);
		glfwTerminate();
#ifdef _WIN32
		MessageBoxA(nullptr, "Failed to load OpenGL", "", MB_OK | MB_ICONERROR);
#else
		std::cerr << "Failed to load OpenGL" << std::endl;
#endif // _WIN32
		return EXIT_FAILURE;
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
	constexpr std::chrono::seconds one(1);
	int tps = 64;
	std::cout << "Type `help` for a list of commands." << std::endl;
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
	bool unsaved = false;
	bool first = true;
	bool randomize = true;
	while (!glfwWindowShouldClose(window)) {
		command = false;
		bool preview = false;
		while (1) {
			glfwPollEvents();
			std::string cmd;
			std::cout << "> ";
			std::cin >> cmd;
			cmd = cmd.substr(cmd.find_first_not_of(" \f\n\r\t\v"));
			std::transform(cmd.begin(), cmd.end(), cmd.begin(),
				[](char c) { return std::tolower(c); });
			std::vector<object>* current = &res::loader::levels.data()[game::level].objects;
			if (cmd.empty())
				continue;
			static constexpr object::enum_type toggleable[] = {
				object::enum_type::DOOR, object::enum_type::MIRROR_DOOR,
				object::enum_type::MOVING_WALL, object::enum_type::MOVING_BLOCK, object::enum_type::MOVING_CRYSTAL
			};
			static const object::enum_type* end = toggleable + sizeof(toggleable) / sizeof(object::enum_type);
			static constexpr std::streamsize stream_max = std::numeric_limits<std::streamsize>::max();
			if (cmd == "help") {
				std::cout << "All parameters in [square brackets] are integers.\n";
				std::cout << "All parameters in (parentheses) are strings.\n";
				std::cout << "All parameters in {curly braces} are floating-point (decimal) numbers.\n";
				std::cout << "List of commands:\n";
				std::cout << "help          - Displays this list.\n";
				std::cout << "cheatsheet    - Displays the numerical values of directions, orientations, and object types.\n";
				std::cout << "load (path)   - Loads a level from a file for editing.\n";
				std::cout << "import (path) - Loads a level from a file for editing.\n";
				std::cout << "export (path) - Exports the level to a file. You can export the default level to inspect the file format.\n";
				std::cout << "                (Note: The four strange integers at the beginning of every level is the RGB value of the\n";
				std::cout << "                 background colour and the noise, type punned from `float` to `int` for storage purposes.)\n";
				std::cout << "save          - Like `export`, but does not warn before overwriting. Must have used `load`/`import`/`export` first.\n";
				std::cout << "name (str)    - Sets the name of the map to (str).\n";
				std::cout << "levels        - Lists all levels as well as their object counts.\n";
				std::cout << "level [n]     - Selects the [n]th level. If this level does not exist, it will be created.\n";
				std::cout << "remove [n]    - Removes the [n]th level.\n";
				std::cout << "text (str)    - Sets the text to be displayed before loading the selected level to (str). Does not support newlines.\n";
				std::cout << "append (str)  - Appends (str) to the text as set by the `text` command **on a new line**.\n";
				std::cout << "rmtext        - Removes the text displayed before the selected level.\n";
				std::cout << "background {r} {g} {b} {noise}\n";
				std::cout << "              - Sets the background colour with RGB value ({r}, {g}, {b}) (default: (0.08, 0.08, 0.08))\n";
				std::cout << "                and noise level {noise} (default: 5). {r}, {g}, {b} are all bound by [0, 1].\n";
				std::cout << "list          - Lists all objects in the selected level.\n";
				std::cout << "add [type] [x] [y] [o]\n";
				std::cout << "              - Places an object of type [type] at ([x], [y]) with orientation [o]. See `cheatsheet`.\n";
				std::cout << "move [id] [x] [y] [o]\n";
				std::cout << "              - Moves the object specified by [id] to coordinates ([x], [y]) and orientation [o].\n";
				std::cout << "resize [id] [width] [height]\n";
				std::cout << "size   [id] [width] [height]\n";
				std::cout << "              - Resizes the object specified by [id] to have width [width] and height [height].\n";
				std::cout << "                Only works with WALL, DOOR, MOVING_WALL, FIXED_BLOCK, MOVING_BLOCK, SPDC_CRYSTAL, and MOVING_CRYSTAL.\n";
				std::cout << "random [id] [bool]\n";
				std::cout << "              - If set to 1, the orientation (if rotatable) or state (if toggleable, not movable, and not linked\n";
				std::cout << "                to any other object) of the object with id [id] is randomized at the beginning of every attempt.\n";
				std::cout << "                If set to 0, the orientation is not randomized. The default value is 1 for applicable objects.\n";
				std::cout << "inspect [id]  - Diplays information on the object specified by [id]. Use `list` for a list of IDs.\n";
				std::cout << "delete [id]   - Removes the object specified by [id]. Use `list` for a list of IDs.\n";
				std::cout << "clear         - Removes all objects in the selected level.\n";
				std::cout << "spawn [x] [y] [dir]\n";
				std::cout << "              - Sets the initial position of the photon to ([x], [y]) with initial direction [dir]. See `cheatsheet`.\n";
				std::cout << "randomize [bool]\n";
				std::cout << "              - If set to 1, newly created applicable objects (see the `random` command) will have their orientation\n";
				std::cout << "                randomized at the beginning of each attempt by default. If set to 0, their orientations will not be\n";
				std::cout << "                randomized by default. The default value is 1. The setting is not saved when the editor is closed.\n";
				std::cout << "preview       - Renders the selected level.\n";
				std::cout << "play          - Plays the selected level.\n";
				std::cout << "credits       - Displays credits.\n";
				std::cout << "quit          - Exits the editor.\n";
				std::cout << "exit          - Exits the editor." << std::endl;
			}
			else if (cmd == "cheatsheet") {
				std::cout << " Directions          | Orientations        | Types\n";
				std::cout << "---------------------+---------------------+---------------------\n";
				std::cout << " E         = 0       | N    = 0            | WALL            = 0\n";
				std::cout << " E_ATAN1_3 = 1       | NNW  = 1            | DOOR            = 1\n";
				std::cout << " E_ATAN1_2 = 2       | NW   = 2            | MOVING_WALL     = 2\n";
				std::cout << " NE        = 3       | NWW  = 3            | MIRROR          = 3\n";
				std::cout << " E_ATAN2   = 4       | W    = 4            | DIAGONAL_MIRROR = 4\n";
				std::cout << " E_ATAN3   = 5       | SWW  = 5            | MIRROR_BLOCK    = 5\n";
				std::cout << " N         = 6       | SW   = 6            | MIRROR_DOOR     = 6\n";
				std::cout << " N_ATAN1_3 = 7       | SSW  = 7            | GLASS_BLOCK     = 7\n";
				std::cout << " N_ATAN1_2 = 8       | S    = 8            | FIXED_BLOCK     = 8\n";
				std::cout << " NW        = 9       | SSE  = 9            | MOVING_BLOCK    = 9\n";
				std::cout << " N_ATAN2   = 10      | SE   = 10           | PRISM           = 10\n";
				std::cout << " N_ATAN3   = 11      | SEE  = 11           | SPDC_CRYSTAL    = 11\n";
				std::cout << " W         = 12      | E    = 12           | MOVING_CRYSTAL  = 12\n";
				std::cout << " W_ATAN1_3 = 13      | NEE  = 13           | SPLITTER        = 13\n";
				std::cout << " W_ATAN1_2 = 14      | NE   = 14           | BOMB            = 14\n";
				std::cout << " SW        = 15      | NNE  = 15           | SENSOR          = 15\n";
				std::cout << " W_ATAN2   = 16      | NONE = 16           | NONE            = 16\n";
				std::cout << " W_ATAN3   = 17      |                     |\n";
				std::cout << " S         = 18      |                     |\n";
				std::cout << " S_ATAN1_3 = 19      |                     |\n";
				std::cout << " S_ATAN1_2 = 20      |                     |\n";
				std::cout << " SW        = 21      |                     |\n";
				std::cout << " S_ATAN2   = 22      |                     |\n";
				std::cout << " S_ATAN3   = 23      |                     |\n";
				std::cout << " NONE      = 24      |                     |" << std::endl;
			}
			else if (cmd == "load" || cmd == "import") {
				std::string path;
				std::getline(std::cin, path);
				if (path.empty()) {
					std::cout << "No path specified" << std::endl;
					continue;
				}
				path = path.substr(1);
				if (unsaved) {
					char c;
					do {
						std::cout << "Save current level? (Y/N) ";
						std::string s;
						std::cin >> s;
						c = std::toupper(s.front());
					} while (c != 'Y' && c != 'N');
					if (c == 'Y') {
						if (game::save.empty()) {
							std::cin.clear();
							std::cin.ignore(stream_max, '\n');
							std::cout << "Enter a filename: ";
							std::getline(std::cin, game::save);
						}
						else {
							std::cin.clear();
							std::cin.ignore(stream_max, '\n');
						}
						if (!save_to_file(game::save)) {
							std::cout << "Something went wrong while writing to the file." << std::endl;
							continue;
						}
						else
							unsaved = false;
					}
					else {
						std::cin.clear();
						std::cin.ignore(stream_max, '\n');
					}
				}
				if (fs::is_regular_file(path)) {
					if (!res::loader::load_from_file(path))
						std::cout << "Failed to read level file" << std::endl;
					else
						game::save = path;
				}
				else
					std::cout << "File not found" << std::endl;
				continue;
			}
			else if (cmd == "export") {
				std::string path;
				std::getline(std::cin, path);
				if (path.empty()) {
					std::cout << "No path specified" << std::endl;
					continue;
				}
				path = path.substr(1);
				if (fs::is_regular_file(path)) {
					char c;
					do {
						std::cout << "The file at " << path << " already exists. Would you like to overwrite it? (Y/N) ";
						std::string s;
						std::cin >> s;
						c = std::toupper(s.front());
					} while (c != 'Y' && c != 'N');
					if (c == 'N') {
						std::cin.clear();
						std::cin.ignore(stream_max, '\n');
						continue;
					}
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
				}
				game::save = path;
				if (!save_to_file(game::save))
					std::cout << "Something went wrong while writing to the file." << std::endl;
				else
					unsaved = false;
				continue;
			}
			else if (cmd == "save") {
				if (game::save.empty()) {
					std::cout << "No level file loaded. Try `export`." << std::endl;
					continue;
				}
				if (!save_to_file(game::save))
					std::cout << "Something went wrong while writing to the file." << std::endl;
				else
					unsaved = false;
			}
			else if (cmd == "name") {
				std::string name;
				std::getline(std::cin, name);
				if (name.empty())
					game::name.clear();
				else
					game::name = name.substr(1);
				unsaved = true;
				continue;
			}
			else if (cmd == "levels") {
				std::cout << game::name << '\n';
				for (int i = 0; i < res::loader::levels.size(); i++)
					std::cout << "Level " << i + 1 << " - "
						<< res::loader::levels.data()[i].objects.size() - 2
						<< " object" << (res::loader::levels.data()[i].objects.size() == 3 ? "" : "s") << "; \""
						<< res::loader::levels.data()[i].hint << "\"" << std::endl;
			}
			else if (cmd == "level") {
				std::string _n;
				std::cin >> _n;
				int n;
				try {
					n = std::stoi(_n);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (n < 1) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if ((size_t)(n - 1) == res::loader::levels.size()) {
					res::loader::levels.push_back(res::loader::level());
					res::loader::levels.back().objects.push_back(object(0.0, 0.0, 1, 1,
						object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, nullptr,
						(int)photon::enum_direction::E, false));
					res::loader::levels.back().objects.push_back(object(0.0, 0.0, 640, 360,
						object::enum_orientation::NONE, object::enum_type::NONE, nullptr, nullptr, 0, false));
					res::loader::set_background((int)(res::loader::levels.size() - 1),
						res::loader::levels.size() - 1 ? res::loader::background().colour : object::default_colour,
						res::loader::levels.size() - 1 ? res::loader::background().noise : object::default_noise);
					unsaved = true;
				}
				if ((size_t)(n - 1) <= res::loader::levels.size())
					game::level = n - 1;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (n < 1 || n > res::loader::levels.size()) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
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
				unsaved = true;
			}
			else if (cmd == "randomize") {
				std::string _r;
				std::cin >> _r;
				bool r;
				try {
					r = std::stoi(_r) != 0;
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				randomize = r;
			}
			else if (cmd == "text") {
				std::string text;
				std::getline(std::cin, text);
				if (text.empty())
					res::loader::levels.data()[game::level].hint.clear();
				else
					res::loader::levels.data()[game::level].hint = text.substr(1);
				unsaved = true;
				continue;
			}
			else if (cmd == "append") {
				std::string text;
				std::getline(std::cin, text);
				res::loader::levels.data()[game::level].hint += "\n\n";
				if (text.empty())
					continue;
				res::loader::levels.data()[game::level].hint += text.substr(1);
				unsaved = true;
				continue;
			}
			else if (cmd == "rmtext") {
				res::loader::levels.data()[game::level].hint.clear();
				unsaved = true;
			}
			else if (cmd == "background") {
				std::string _r, _g, _b, _noise;
				std::cin >> _r >> _g >> _b >> _noise;
				float r, g, b, noise;
				try {
					r = std::stof(_r);
					g = std::stof(_g);
					b = std::stof(_b);
					noise = std::stof(_noise);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f || noise < 0.0f) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				res::loader::set_background(game::level, vec(r, g, b, 1.0f), noise);
				unsaved = true;
			}
			else if (cmd == "list") {
				const object& p = current->front();
				std::cout << "spawn | x=" << p.x << ", y=" << p.y << ", direction=" << p.data << std::endl;
				for (int i = 2; i < current->size(); i++) {
					const object& obj = current->data()[i];
					std::cout << "id=" << i - 2 << " | type=" << (int)obj.type
						<< ", x=" << obj.x << ", y=" << obj.y
						<< ", orientation=" << (int)obj.orientation
						<< ", otherx=" << obj.x2 << ", othery=" << obj.y2
						<< ", number=" << obj.data << std::endl;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (o < 0 || o > (int)object::enum_orientation::NONE
					|| type < 0 || type > (int)object::enum_type::NONE) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				object::enum_type t = (object::enum_type)type;
				current->push_back(object(x, y, 20, 20,
					(object::enum_orientation)o, t,
					res::loader::get_tex(t), res::loader::get_hbx(t), 0, false));
				if (std::find(toggleable, end, t) != end) {
					int x2, y2, link;
					if (t == object::enum_type::MOVING_WALL
						|| t == object::enum_type::MOVING_BLOCK
						|| t == object::enum_type::MOVING_CRYSTAL) {
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
								std::cin.clear();
								std::cin.ignore(stream_max, '\n');
							}
						}
					}
					while (1) {
						try {
							std::cout << "Enter a number. All toggleable objects with the same number (except 0) will be activated in tandem. ";
							std::string _link;
							std::cin >> _link;
							link = std::stoi(_link);
							if (link < 0) {
								std::cout << "Invalid input" << std::endl;
								std::cin.clear();
								std::cin.ignore(stream_max, '\n');
							}
							else
								break;
						}
						catch (...) {
							std::cout << "Invalid input" << std::endl;
							std::cin.clear();
							std::cin.ignore(stream_max, '\n');
						}
					}
					if (t == object::enum_type::MOVING_WALL
						|| t == object::enum_type::MOVING_BLOCK
						|| t == object::enum_type::MOVING_CRYSTAL) {
						current->back().x2 = x2;
						current->back().y2 = y2;
					}
					current->back().data = link;
				}
				current->back().randomize = randomize
					&& (current->back().type == object::enum_type::MIRROR || current->back().type == object::enum_type::GLASS_BLOCK
						|| (!current->back().data && std::find(toggleable, end, current->back().type) != end));
				unsaved = true;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()) {
					std::cout << "ID out of range" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (o < 0 || o > (int)object::enum_orientation::NONE) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				current->data()[id].x = x;
				current->data()[id]._x = x;
				current->data()[id].y = y;
				current->data()[id]._y = y;
				current->data()[id].orientation = (object::enum_orientation)o;
				unsaved = true;
			}
			else if (cmd == "resize" || cmd == "size") {
				std::string _id, _w, _h;
				std::cin >> _id >> _w >> _h;
				int id, w, h;
				try {
					id = std::stoi(_id);
					w = std::stoi(_w);
					h = std::stoi(_h);
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()) {
					std::cout << "ID out of range" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				static constexpr object::enum_type types[] = {
					object::enum_type::WALL, object::enum_type::DOOR, object::enum_type::FIXED_BLOCK, object::enum_type::SPDC_CRYSTAL,
					object::enum_type::MOVING_WALL, object::enum_type::MOVING_BLOCK, object::enum_type::MOVING_CRYSTAL
				};
				static const object::enum_type* t_end = types + sizeof(types) / sizeof(object::enum_type);
				if (std::find(types, t_end, current->data()[id].type) == t_end) {
					std::cout << "This object cannot be resized.";
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (w < 10 || h < 10)
					std::cout << "Note: Objects smaller than 10 by 10 may look strange" << std::endl;
				current->data()[id].width = w;
				current->data()[id].height = h;
				unsaved = true;
			}
			else if (cmd == "random") {
				std::string _id, _r;
				std::cin >> _id >> _r;
				int id;
				bool r;
				try {
					id = std::stoi(_id);
					r = std::stoi(_r) != 0;
				}
				catch (...) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()) {
					std::cout << "ID out of range" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				object& obj = current->data()[id];
				if (r) {
					if ((obj.type == object::enum_type::MIRROR || obj.type == object::enum_type::GLASS_BLOCK
						|| (!obj.data && std::find(toggleable, end, obj.type) != end)))
						obj.randomize = true;
					else
						std::cout << "`random` is not applicable to this object." << std::endl;
				}
				else
					obj.randomize = false;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				id += 2;
				if (id < 2 || id >= current->size()) {
					std::cout << "ID out of range" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				const object& obj = current->data()[id];
				std::cout << "type=" << (int)obj.type
					<< ", x=" << obj.x << ", y=" << obj.y
					<< ", orientation=" << (int)obj.orientation
					<< ", otherx=" << obj.x2 << ", othery=" << obj.y2
					<< ", number=" << obj.data
					<< ", randomize=" << (int)obj.randomize << std::endl;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (id < 0 || id >= current->size() - 2) {
					std::cout << "ID out of range" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				current->erase(current->begin() + (id + 2));
				unsaved = true;
			}
			else if (cmd == "clear") {
				if (current->empty()) {
					std::cout << "Nothing to clear." << std::endl;
					continue;
				}
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
						object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, nullptr,
						(int)photon::enum_direction::E, false));
					current->push_back(object(0.0, 0.0, 640, 360,
						object::enum_orientation::NONE, object::enum_type::NONE, nullptr, nullptr, 0, false));
				}
				unsaved = true;
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
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				if (dir >= (int)photon::enum_direction::NONE) {
					std::cout << "Invalid input" << std::endl;
					std::cin.clear();
					std::cin.ignore(stream_max, '\n');
					continue;
				}
				current->data()[0].x = x;
				current->data()[0].y = y;
				current->data()[0].data = dir;
				unsaved = true;
			}
			else if (cmd == "preview") {
				res::loader::load_level(game::level, false);
				preview = true;
				glfwFocusWindow(window);
				break;
			}
			else if (cmd == "play") {
				std::cout << "Press ESC to return to the console." << std::endl;
				if (first) {
					first = false;
					std::this_thread::sleep_for(one);
					// Sleep so that the user has time to read the prompt
				}
				res::loader::load_level(game::level, false);
				glfwFocusWindow(window);
				break;
			}
			else if (cmd == "credits") {
				std::cout << "This game uses the following libraries:\n";
				std::cout << "GLFW: Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla Löwy; licensed under the zlib License\n";
				std::cout << "glad: Created by David Herberth; in the public domain" << std::endl;
			}
			else if (cmd == "quit" || cmd == "exit") {
				if (unsaved) {
					char c;
					do {
						std::cout << "Save? (Y/N) ";
						std::string s;
						std::cin >> s;
						c = std::toupper(s.front());
					} while (c != 'Y' && c != 'N');
					if (c == 'Y') {
						if (game::save.empty()) {
							std::cin.clear();
							std::cin.ignore(stream_max, '\n');
							std::cout << "Enter a filename: ";
							std::getline(std::cin, game::save);
						}
						if (!save_to_file(game::save)) {
							std::cout << "Something went wrong while writing to the file." << std::endl;
							continue;
						}
					}
				}
				glfwDestroyWindow(window);
				glfwTerminate();
				return EXIT_SUCCESS;
			}
			else
				std::cout << "Unrecognized command" << std::endl;
			std::cin.clear();
			std::cin.ignore(stream_max, '\n');
		}
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
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
				int fails = game::failures, level = game::level;
				for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();) {
					it->tick(tps, len, &it);
					if (game::level > level || game::failures > fails)
						break;
				}
				if (game::level > level) {
					game::level--;
					break;
				}
				else if (game::failures == fails) {
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
					if ((game::frame % 10 - game::frame % 2) && !object::invalidate_all) {
						for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end(); ++it) {
							glEnable(GL_SCISSOR_TEST);
							glScissor(
								(GLint)std::round(it->_x - 480.0 / tps) * PX_SIZE, 40 + (GLint)std::round(it->_y - 480.0 / tps) * PX_SIZE,
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
									return obj.type != object::enum_type::NONE
										&& object::overlapping(obj, *other);
								}))
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
	else if (key == GLFW_KEY_ESCAPE)
		command = true;
}

bool save_to_file(std::string path) {
	std::ofstream out(path, std::ios::trunc);
	out << game::name << '\n';
	for (int i = 0; i < res::loader::levels.size(); i++) {
		std::string& hint = res::loader::levels.data()[i].hint;
		out << ' ' << std::count(hint.begin(), hint.end(), '\n') + (hint.empty() ? 0 : 1);
		if (!hint.empty())
			out << '\n';
		out << hint << '\n';
		const object& p = res::loader::levels.data()[i].objects.data()[0];
		const rect& background = res::loader::background(i);
		out << p.x << ' ' << p.y << ' ' << p.data << ' '
			<< p.x1 << ' ' << p.x2 << ' ' << p.y1 << ' ' << p.y2 << '\n';
		for (int j = 2; j < res::loader::levels.data()[i].objects.size(); j++) {
			const object& obj = res::loader::levels.data()[i].objects.data()[j];
			out << (int)obj.type << ' ' << obj.x << ' ' << obj.y << ' '
				<< obj.width << ' ' << obj.height << ' '
				<< (int)obj.orientation << ' ' << (int)obj.randomize;
			if (obj.type == object::enum_type::MOVING_WALL
				|| obj.type == object::enum_type::MOVING_BLOCK
				|| obj.type == object::enum_type::MOVING_CRYSTAL)
				out << ' ' << obj.x2 << ' ' << obj.y2 << ' ' << obj.data;
			else if (obj.type == object::enum_type::DOOR || obj.type == object::enum_type::MIRROR_DOOR)
				out << ' ' << (int)obj.toggle << ' ' << obj.data;
			out << '\n';
		}
	}
	std::flush(out);
	bool success = !out.fail();
	out.close();
	return success;
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
