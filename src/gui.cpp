#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>

#include "gui.hpp"

#define FONT_SPACEBAR	(char)16
#define FONT_CURSOR 	(char)17
#define FONT_TCOLON 	(char)18
#define FONT_TDECIMAL	(char)19
#define FONT_O_UMLAUT	(char)20

#define STR(x) std::string({(char)x})

namespace fs = std::filesystem;

GLFWwindow* gui::window;

namespace gui {
	bool menu = true;
	bool quit = false;
	std::vector<element*> elements;
	std::vector<rect> font[128];
}

void gui::load_gui(void) {
	const vec fore = vec(1.0f, 1.0f, 1.0f, 1.0f);
	const vec back = vec(0.2f, 0.2f, 0.2f, 0.4f);
	gui::elements.push_back(new gui::panel(0, 0, 1280, 720, true, vec(0.1f, 0.1f, 0.1f, 1.0f), 6.0f));
	gui::elements.push_back(new gui::label(100, 100, 1000, true, "PHOTON", fore, 6));
	gui::elements.push_back(new gui::button(100, 240, 90, 42, true, 10, "PLAY",
		fore, back, []() {
		if (game::hardcore && game::failures < 0)
			return;
		if (game::level >= res::loader::levels.size())
			return;
		if (game::started) {
			glClear(GL_COLOR_BUFFER_BIT);
			int index = 0;
			for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "No valid savefile loaded"; index++);
			gui::elements.data()[index]->visible = false;
			gui::elements.data()[index + 1]->visible = false;
			game::frame = -1;
			gui::menu = false;
		}
		else if (!game::save.empty()) {
			std::ofstream out(game::save, std::ios::app);
			out << std::endl;
			if (out.fail()) {
				int index = 0;
				for (; index < gui::elements.size() - 1 && gui::elements.data()[index]->text != "Couldn't write to savefile"; index++);
				gui::elements.data()[index]->visible = true;
				gui::elements.data()[index - 1]->visible = false;
			}
			else {
				glClear(GL_COLOR_BUFFER_BIT);
				gui::elements.data()[1]->text = "PAUSED";
				game::started = true;
				game::frame = -1;
				gui::menu = false;
				res::loader::load_level(game::level, true);
			}
			out.close();
		}
		else {
			int index = 0;
			for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "No valid savefile loaded"; index++);
			gui::elements.data()[index]->visible = true;
			gui::elements.data()[index + 1]->visible = false;
		}
	}));
	gui::elements.push_back(new gui::button(100, 300, 160, 42, true, 10, "KEYBINDS",
		fore, back, []() {
		int index = 0;
		for (; index < gui::elements.size() - 1 && gui::elements.data()[index]->text != "<BACK"; index++);
		for (int i = 1; i < index; i++)
			gui::elements[i]->visible = false;
		for (int i = index; i < index + 21; i++)
			gui::elements[i]->visible = true;
		for (int i = index + 21; i < gui::elements.size(); i++)
			gui::elements[i]->visible = false;
		if (keybinds::up == 0 || keybinds::left == 0 || keybinds::down == 0 || keybinds::right == 0
			|| keybinds::ccw == 0 || keybinds::cw == 0 || keybinds::perp == 0 || keybinds::toggle == 0
			|| keybinds::hint == 0) {
			index = 0;
			for (; index < gui::elements.size() - 1
				&& gui::elements.data()[index]->text.find("Missing keybinds") == std::string::npos; index++);
			gui::elements.data()[index]->visible = true;
		}
	}));
	gui::elements.push_back(new gui::label(105, 360, 1000, true, "Path to savefile:", fore, 2));
	gui::elements.push_back(new gui::textbox(100, 390, 1080, 140, true, 10, game::save, fore, back));
	gui::elements.push_back(new gui::button(100, 540, 180, 42, true, 10, "LOAD SAVE",
		fore, back, []() {
		int index = 0;
		for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "Path to savefile:"; index++);
		std::string save = gui::elements.data()[index + 1]->text;
		if (!save.empty() && fs::is_regular_file(save)) {
			std::ifstream in(save);
			char hc;
			in >> hc;
			game::hardcore = hc == 'h';
			in >> game::level >> game::failures >> game::time;
			if (in.fail()) {
				int index = 0;
				for (; index < gui::elements.size() - 1 && gui::elements.data()[index]->text != "Invalid savefile"; index++);
				gui::elements.data()[index]->visible = true;
				gui::elements.data()[index - 1]->visible = false;
			}
			else {
				int index = 0;
				for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "File not found"; index++);
				gui::elements.data()[index]->visible = false;
				gui::elements.data()[index + 1]->visible = false;
				game::save = fs::absolute(fs::path(save)).string();
				std::ofstream cfg("./settings", std::ios::trunc);
				cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
				cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
				cfg << keybinds::hint << '\n' << game::save << std::endl;
				cfg.close();
			}
			in.close();
		}
		else {
			int index = 0;
			for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "File not found"; index++);
			gui::elements.data()[index]->visible = true;
			gui::elements.data()[index + 1]->visible = false;
		}
	}));
	gui::elements.push_back(new gui::button(555, 540, 290, 42, true, 10, "NEW NORMAL SAVE",
		fore, back, []() {
		if (!fs::is_directory("./saves"))
			fs::create_directory("./saves");
		size_t i = 0;
		for (; i < SIZE_MAX; i++)
			if (!fs::is_regular_file(fs::path("./saves") / (std::to_string(i) + ".save")))
				break;
		std::ofstream out(fs::path("./saves") / (std::to_string(i) + ".save"));
		out << "n\n0\n0\n0" << std::endl;
		out.close();
		game::hardcore = false;
		game::level = 0;
		game::failures = 0;
		game::time = 0;
		game::save = fs::absolute(fs::path("./saves") / (std::to_string(i) + ".save")).string();
		int index = 0;
		for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "Path to savefile:"; index++);
		gui::elements.data()[index + 1]->text = game::save;
		std::ofstream cfg("./settings", std::ios::trunc);
		cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
		cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
		cfg << keybinds::hint << '\n' << game::save << std::endl;
		cfg.close();
	}));
	gui::elements.push_back(new gui::button(855, 540, 325, 42, true, 10, "NEW HARDCORE SAVE",
		fore, back, []() {
		if (!fs::is_directory("./saves"))
			fs::create_directory("./saves");
		size_t i = 0;
		for (; i < SIZE_MAX; i++)
			if (!fs::is_regular_file(fs::path("./saves") / (std::to_string(i) + ".save")))
				break;
		std::ofstream out(fs::path("./saves") / (std::to_string(i) + ".save"));
		out << "h\n0\n0\n0" << std::endl;
		out.close();
		game::hardcore = true;
		game::level = 0;
		game::failures = 0;
		game::time = 0;
		game::save = fs::absolute(fs::path("./saves") / (std::to_string(i) + ".save")).string();
		int index = 0;
		for (; index < gui::elements.size() - 2 && gui::elements.data()[index]->text != "Path to savefile:"; index++);
		gui::elements.data()[index + 1]->text = game::save;
		std::ofstream cfg("./settings", std::ios::trunc);
		cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
		cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
		cfg << keybinds::hint << '\n' << game::save << std::endl;
		cfg.close();
	}));
	gui::elements.push_back(new gui::button(100, 600, 90, 42, true, 10, "QUIT",
		fore, back, []() { gui::quit = true; }));
	gui::elements.push_back(new gui::button(1035, 600, 145, 42, true, 10, "CREDITS",
		fore, back, []() {
		static int count = 0;
		int index = 0;
		for (; index < gui::elements.size() - 1
			&& gui::elements.data()[index]->text.find("License") == std::string::npos; index++);
		gui::elements.data()[index]->visible = !gui::elements.data()[index]->visible;
		if (++count > 4 && !mix32_rand(3)) {
			index = 0;
			for (; index < gui::elements.size() - 1
				&& gui::elements.data()[index]->text.find("neondev1") == std::string::npos; index++);
			gui::elements.data()[index]->visible = true;
		}
	}));
	// Keybinds menu
	gui::elements.push_back(new gui::button(20, 20, 110, 42, false, 10, "<BACK",
		fore, back, []() {
		int index = 0;
		for (; index < gui::elements.size() - 1 && gui::elements.data()[index]->text != "<BACK"; index++);
		for (int i = 1; i < index; i++)
			gui::elements[i]->visible = true;
		for (int i = index; i < gui::elements.size(); i++)
			gui::elements[i]->visible = false;
	}));
	gui::elements.push_back(new gui::label(160, 25, 1000, false, "Keybinds", fore, 3));
	gui::elements.push_back(new gui::label(50, 100, 1000, false, "Up", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 90, false, &keybinds::up, fore, back));
	gui::elements.push_back(new gui::label(50, 150, 1000, false, "Left", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 140, false, &keybinds::left, fore, back));
	gui::elements.push_back(new gui::label(50, 200, 1000, false, "Down", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 190, false, &keybinds::down, fore, back));
	gui::elements.push_back(new gui::label(50, 250, 1000, false, "Right", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 240, false, &keybinds::right, fore, back));
	gui::elements.push_back(new gui::label(50, 300, 1000, false, "Rotate Counterclockwise", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 290, false, &keybinds::ccw, fore, back));
	gui::elements.push_back(new gui::label(50, 350, 1000, false, "Rotate Clockwise", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 340, false, &keybinds::cw, fore, back));
	gui::elements.push_back(new gui::label(50, 400, 1000, false, "Rotate 90 Degrees", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 390, false, &keybinds::perp, fore, back));
	gui::elements.push_back(new gui::label(50, 450, 1000, false, "Toggle/Move Object", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 440, false, &keybinds::toggle, fore, back));
	gui::elements.push_back(new gui::label(50, 500, 1000, false, "Hint", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 490, false, &keybinds::hint, fore, back));
	gui::elements.push_back(new gui::button(400, 20, 180, 42, false, 10, "RESET ALL", fore, back, []() {
		keybinds::up    	= GLFW_KEY_W;
		keybinds::left  	= GLFW_KEY_A;
		keybinds::down  	= GLFW_KEY_S;
		keybinds::right 	= GLFW_KEY_D;
		keybinds::ccw   	= GLFW_KEY_LEFT_BRACKET;
		keybinds::cw    	= GLFW_KEY_RIGHT_BRACKET;
		keybinds::perp  	= GLFW_KEY_BACKSLASH;
		keybinds::toggle	= GLFW_KEY_SLASH;
		keybinds::hint  	= GLFW_KEY_H;
		int index = 0;
		for (; index < gui::elements.size() - 1
			&& gui::elements.data()[index]->text.find("Missing keybinds") == std::string::npos; index++);
		gui::elements.data()[index]->visible = false;
		std::ofstream cfg("./settings", std::ios::trunc);
		cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
		cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
		cfg << keybinds::hint << '\n' << game::save << std::endl;
		cfg.close();
	}));
	// Messages
	gui::elements.push_back(new gui::label(1010, 120, 270, false, "      Matt was here\ngithub" + STR(FONT_TDECIMAL) + "com/neondev1", fore, 1));
	gui::elements.push_back(new gui::label(200, 255, 1000, false, "No valid savefile loaded", fore, 1));
	gui::elements.push_back(new gui::label(200, 255, 1000, false, "Unable to write to savefile", fore, 1));
	gui::elements.push_back(new gui::label(290, 555, 1000, false, "File not found", fore, 1));
	gui::elements.push_back(new gui::label(290, 555, 1000, false, "Invalid savefile", fore, 1));
	gui::elements.push_back(new gui::label(100, 660, 1000, false, "This game uses the following libraries:\nGLFW: Copyright (c) 2002-2006 Marcus Geelnard, (c) 2006-2019 Camilla L" + STR(FONT_O_UMLAUT) + "wy; licensed under the zlib License\nglad: Created by David Herberth; in the public domain", fore, 1));
	gui::elements.push_back(new gui::label(700, 100, 1000, false, "Fails: ", fore, 2));
	gui::elements.push_back(new gui::label(700, 100, 1000, false, "Level: ", fore, 2));
	gui::elements.push_back(new gui::label(718, 144, 1000, false, "Time: ", fore, 2));
	gui::elements.push_back(new gui::label(50, 550, 1000, false, "Warning: Missing keybinds", vec(1.0f, 0.8f, 0.3f, 1.0f), 2));
}

std::string gui::time(double t) {
	std::ostringstream oss;
	long long hours = (long long)(t / 3600.0);
	int minutes = (int)((t - hours * 3600.0) / 60.0);
	int seconds = (int)(t - hours * 3600.0 - minutes * 60.0);
	int millis = (int)((t - hours * 3600.0 - minutes * 60.0 - seconds) * 1000.0);
	oss << hours << FONT_TCOLON
		<< std::setfill('0') << std::setw(2) << minutes << FONT_TCOLON
		<< std::setw(2) << seconds << FONT_TDECIMAL
		<< std::setw(3) << millis;
	return oss.str();
}

static void draw(int c, int x, int y, int size, vec colour) {
	std::vector<rect>* tex;
	if (c >= 'a' && c <= 'z')
		tex = &gui::font[c - 'a' + 'A'];
	else if (c < 0)
		tex = &gui::font[0];
	else if (c > 127) {
		switch (c) {
		case GLFW_KEY_RIGHT:    	tex = &gui::font[1];	break;
		case GLFW_KEY_LEFT:     	tex = &gui::font[2];	break;
		case GLFW_KEY_DOWN:     	tex = &gui::font[3];	break;
		case GLFW_KEY_UP:        	tex = &gui::font[4];	break;
		case GLFW_KEY_LEFT_SHIFT:	tex = &gui::font[5];	break;
		case GLFW_KEY_RIGHT_SHIFT:	tex = &gui::font[6];	break;
		case GLFW_KEY_CAPS_LOCK:	tex = &gui::font[7];	break;
		case GLFW_KEY_BACKSPACE:	tex = &gui::font['\b'];	break;
		case GLFW_KEY_TAB:      	tex = &gui::font['\t'];	break;
		case GLFW_KEY_ENTER:    	tex = &gui::font['\r'];	break;
		default:                	tex = &gui::font[0];	break;
		}
	}
	else if (gui::font[c].size() == 0 && c != ' ')
		tex = &gui::font[0];
	else
		tex = &gui::font[c];
	for (int i = 0; i < tex->size(); i++) {
		rect& r = tex->data()[i];
		glEnable(GL_SCISSOR_TEST);
		glScissor(x + r.x * size, y + r.y * size, r.width * size, r.height * size);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
			1, colour.ptr());
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_SCISSOR_TEST);
	}
}

void render_text(std::string text, int x, int y, int width, int size, bool cursor, size_t cursor_pos, vec colour) {
	if (width == 0)
		return;
	GLint gl_x = x, gl_y = 720 - y - size * 11;
	width = (width + size * 2) / (size * 9);
	GLint xpos = 0, ypos = 0;
	size_t prev = 0;
	if (cursor_pos == 0 && cursor)
		draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
	for (size_t cur;
		(cur = text.find_first_of(" \t/\\\r\n-()[]{}", prev)) != std::string::npos;
		prev = cur + 1) {
		std::string token = text.substr(prev, cur - prev);
		if ((token.length() <= width && xpos + token.length() > width)
			|| xpos >= width) {
			xpos = 0;
			ypos++;
		}
		for (int i = 0; i < cur - prev; i++) {
			draw(token.data()[i], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			if (++xpos >= width) {
				xpos = 0;
				ypos++;
			}
			if (prev + i + 1 == cursor_pos && cursor)
				draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
		}
		if (text.data()[cur] == '\r' || text.data()[cur] == '\n') {
			xpos = 0;
			ypos++;
			if (cur + 1 == cursor_pos && cursor)
				draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
		}
		else if (text.data()[cur] != ' ' || xpos != 0) {
			draw(text.data()[cur], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			if (++xpos >= width) {
				xpos = 0;
				ypos++;
			}
			if (cur + 1 == cursor_pos && cursor)
				draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
		}
		else if (cur + 1 == cursor_pos && cursor)
			draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
	}
	if (prev < text.length()) {
		std::string token = text.substr(prev);
		if (token.length() <= width && xpos + token.length() > width
			|| xpos >= width) {
			xpos = 0;
			ypos++;
		}
		for (int i = 0; i < token.length(); i++) {
			draw(token.data()[i], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			if (++xpos >= width) {
				xpos = 0;
				ypos++;
			}
			if (prev + i + 1 == cursor_pos && cursor)
				draw(FONT_CURSOR, gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
		}
	}
}

gui::element::element(int x, int y, int width, bool visible,
	std::string text, vec foreground) :
	x(x), y(y), width(width), text(text), visible(visible), foreground(foreground) {}

// panel

gui::panel::panel(int x, int y, int width, int height, bool visible,
	vec colour, GLfloat noise) :
	element(x, y, width, visible, "", colour), height(height), noise(noise), offset(mix32_rand(0x80000000U)) { }

void gui::panel::render(void) {
	if (!visible)
		return;
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 720 - y - height, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
		1, foreground.ptr());
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), noise);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 3.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
}

// label

gui::label::label(int x, int y, int width, bool visible,
	std::string text, vec foreground, int size) :
	element(x, y, width, visible, text, foreground), size(size) {}

void gui::label::render(void) {
	if (visible)
		render_text(text, x, y, width, size, false, 0, foreground);
}

// button

gui::button::button(int x, int y, int width, int height,
	bool visible, int margin, std::string text,
	vec foreground, vec background, void (*event)(void)) :
	element(x, y, width, visible, text, foreground), height(height), margin(margin),
	state(0), inactive(background), event(event) {
	hover = vec(background.r * 1.2f, background.g * 1.2f, background.b * 1.2f, background.a);
	click = vec(background.r * 1.4f, background.g * 1.4f, background.b * 1.4f, background.a);
}

void gui::button::handler(enum_event type, int a, int b, int c) {
	if (!visible)
		return;
	switch (type) {
	case enum_event::MOVE:
		if (a < x || a > x + width || b < y || b > y + height)
			state = 0;
		else if (state == 0)
			state = 1;
		break;
	case enum_event::CLICK:
		if (a != GLFW_MOUSE_BUTTON_1)
			break;
		if (state == 1 && b == GLFW_PRESS)
			state = 2;
		else if (state == 2 && b == GLFW_RELEASE) {
			if (event)
				event();
			state = 1;
		}
		break;
	default:
		break;
	}
}

void gui::button::render(void) {
	if (!visible)
		return;
	vec colour;
	switch (state) {
	case 0: colour = inactive;	break;
	case 1: colour = hover;		break;
	case 2: colour = click;		break;
	default:					break;
	}
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 720 - y - height, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
		1, colour.ptr());
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	render_text(text, x + margin, y + margin, width - 2 * margin, 2, false, 0, foreground);
}

// keybind_button

gui::keybind_button::keybind_button(int x, int y, bool visible, int* key,
	vec foreground, vec background) :
	button(x, y, 34, 42, visible, 10, "", foreground, background, nullptr),
	key(key) {}

void gui::keybind_button::handler(enum_event type, int a, int b, int c) {
	if (!visible)
		return;
	switch (type) {
	case enum_event::MOVE:
		if (state == 2)
			break;
		state = (a < x || a > x + width || b < y || b > y + height) ? 0 : 1;
		break;
	case enum_event::CLICK:
		if (a == GLFW_MOUSE_BUTTON_1) {
			double _x = 0, _y = 0;
			glfwGetCursorPos(gui::window, &_x, &_y);
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 2;
		}
		break;
	case enum_event::KEY:
		if (b == GLFW_PRESS && state == 2) {
			if (a != GLFW_KEY_ESCAPE) {
				*key = a;
				bool flag = false;
#define CHECK_KEYBIND(bind, code, ptr, flag) do { if ((keybinds::bind == code && &keybinds::bind != ptr) || keybinds::bind == 0) { keybinds::bind = 0; flag = true; } } while (0)
				CHECK_KEYBIND(up, a, key, flag);
				CHECK_KEYBIND(left, a, key, flag);
				CHECK_KEYBIND(down, a, key, flag);
				CHECK_KEYBIND(right, a, key, flag);
				CHECK_KEYBIND(ccw, a, key, flag);
				CHECK_KEYBIND(cw, a, key, flag);
				CHECK_KEYBIND(perp, a, key, flag);
				CHECK_KEYBIND(toggle, a, key, flag);
				CHECK_KEYBIND(hint, a, key, flag);
#undef CHECK_KEYBIND
				int index = 0;
				for (; index < gui::elements.size() - 1
					&& gui::elements.data()[index]->text.find("Missing keybinds") == std::string::npos; index++);
				gui::elements.data()[index]->visible = flag;
				std::ofstream cfg("./settings", std::ios::trunc);
				cfg << keybinds::up << '\n' << keybinds::left << '\n' << keybinds::down << '\n' << keybinds::right << '\n';
				cfg << keybinds::ccw << '\n' << keybinds::cw << '\n' << keybinds::perp << '\n' << keybinds::toggle << '\n';
				cfg << keybinds::hint << '\n' << game::save << std::endl;
				cfg.close();
			}
			double _x = 0, _y = 0;
			glfwGetCursorPos(gui::window, &_x, &_y);
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 1;
		}
		break;
	default:
		break;
	}
}

void gui::keybind_button::render(void) {
	if (!visible)
		return;
	button::render();
	int c;
	if (*key == 0)
		c = ' ';
	else if (*key == ' ')
		c = FONT_SPACEBAR;
	else
		c = *key;
	draw(c, x + margin, 698 - (y + margin), 2, foreground);
}

// textbox

gui::textbox::textbox(int x, int y, int width, int height,
	bool visible, int margin, std::string text,
	vec foreground, vec background) :
	element(x, y, width, visible, text, foreground), height(height), margin(margin),
	state(0), cursor_pos(0), last_clicked(0.0), inactive(background) {
	active = vec(background.r * 1.4f, background.g * 1.4f, background.b * 1.4f, background.a);
}

void gui::textbox::handler(enum_event type, int a, int b, int c) {
	if (!visible)
		return;
	switch (type) {
	case enum_event::MOVE:
		if (state == 2)
			break;
		state = (a < x || a > x + width || b < y || b > y + height) ? 0 : 1;
		break;
	case enum_event::CLICK:
		if (a == GLFW_MOUSE_BUTTON_1) {
			double _x = 0, _y = 0;
			glfwGetCursorPos(gui::window, &_x, &_y);
			if (state != 2)
				cursor_pos = text.length();
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 2;
			if (state == 2)
				last_clicked = glfwGetTime();
		}
		break;
	case enum_event::TEXT:
		if (state != 2)
			break;
		last_clicked = glfwGetTime();
		if (a <= 0x7f)
			text.insert(cursor_pos++, {(char)a});
		else {
			// this is deprecated but I don't care
			size_t s = text.length();
			unsigned codepoint = (unsigned)a;
			text.insert(cursor_pos, std::wstring_convert<std::codecvt_utf8<unsigned>, unsigned>().to_bytes(&codepoint, &codepoint + 1));
			cursor_pos += text.length() - s;
		}
		break;
	case enum_event::KEY:
		if (b == GLFW_RELEASE || state != 2)
			break;
		switch (a) {
		case GLFW_KEY_C:
			if (c & GLFW_MOD_CONTROL)
				glfwSetClipboardString(nullptr, text.c_str());
			break;
		case GLFW_KEY_V:
			if (c & GLFW_MOD_CONTROL) {
				if (const char* str = glfwGetClipboardString(nullptr)) {
					text.insert(cursor_pos, str);
					cursor_pos += strlen(str);
					last_clicked = glfwGetTime();
				}
			}
			break;
		case GLFW_KEY_X:
			if (c & GLFW_MOD_CONTROL) {
				glfwSetClipboardString(nullptr, text.c_str());
				text.clear();
			}
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_BACKSPACE:
			if (c & GLFW_MOD_CONTROL) {
				size_t index = text.find_last_of("\\/", cursor_pos - 2) + 1;
				text.erase(index < 0 ? 0 : index, cursor_pos - index);
				cursor_pos = index;
			}
			else if (cursor_pos != 0)
				text.erase(--cursor_pos, 1);
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_DELETE:
			if (c & GLFW_MOD_CONTROL) {
				size_t index = text.find_first_of("\\/", cursor_pos);
				index = index == std::string::npos ? text.length() : index + 1;
				text.erase(cursor_pos, index - cursor_pos);
			}
			else if (cursor_pos != text.length())
				text.erase(cursor_pos, 1);
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_LEFT:
			if (c & GLFW_MOD_CONTROL) {
				size_t index = text.find_last_of("\\/", cursor_pos - 2);
				index = index == std::string::npos ? 0 : index + 1;
				cursor_pos = index < 0 ? 0 : index;
			}
			else if (cursor_pos != 0)
				cursor_pos--;
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_RIGHT:
			if (c & GLFW_MOD_CONTROL) {
				size_t index = text.find_first_of("\\/", cursor_pos);
				cursor_pos = index == std::string::npos ? text.length() : index + 1;
			}
			else if (cursor_pos != text.length())
				cursor_pos++;
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_UP:
			cursor_pos = 0;
			last_clicked = glfwGetTime();
			break;
		case GLFW_KEY_DOWN:
			cursor_pos = text.length();
			last_clicked = glfwGetTime();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void gui::textbox::render(void) {
	if (!visible)
		return;
	vec colour = state > 0 ? active : inactive;
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 720 - y - height, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
		1, colour.ptr());
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	bool cursor = state == 2 && (long long)((glfwGetTime() - last_clicked) * 1000.0) % 1060LL < 530LL;
	render_text(text, x + margin, y + margin, width - 2 * margin, 2, cursor, cursor_pos, foreground);
}
