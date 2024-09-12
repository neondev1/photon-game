#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#include <codecvt>
#include <locale>

#include <glm/gtc/type_ptr.hpp>

#include "gui.hpp"

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

namespace gui {
	bool menu = true;
	bool started = false;
	bool quit = false;
	std::vector<element*> elements;
	std::vector<rect> font[128];
}

void gui::load_gui(void) {
	glm::vec4 fore = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 back = glm::vec4(0.2f, 0.2f, 0.2f, 0.4f);
	gui::elements.push_back(new gui::panel(0, 0, 1280, 720, true, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), 6.0f));
	gui::elements.push_back(new gui::label(100, 100, 1000, true, "PHOTON", fore, 6));
	gui::elements.push_back(new gui::button(100, 240, 90, 42, true, 10, "PLAY",
		fore, back, []() {
		if (gui::started)
			gui::menu = false;
		else {
			gui::elements.data()[1]->text = "PAUSED";
			gui::started = true;
			gui::menu = false;
		}
	}));
	gui::elements.push_back(new gui::button(100, 300, 160, 42, true, 10, "KEYBINDS",
		fore, back, []() {
		int index = 0;
		for (; index < gui::elements.size() && gui::elements.data()[index]->text != "<BACK"; index++);
		for (int i = 1; i < index; i++)
			gui::elements[i]->visible = false;
		for (int i = index; i < index + 18; i++)
			gui::elements[i]->visible = true;
	}));
	gui::elements.push_back(new gui::label(105, 360, 1000, true, "Path to savefile:", fore, 2));
	gui::elements.push_back(new gui::textbox(100, 390, 1080, 140, true, 10, "", fore, back));
	gui::elements.push_back(new gui::button(100, 540, 160, 42, true, 10, "NEW SAVE",
		fore, back, NULL));
	gui::elements.push_back(new gui::button(270, 540, 180, 42, true, 10, "LOAD SAVE",
		fore, back, NULL));
	gui::elements.push_back(new gui::button(100, 600, 90, 42, true, 10, "QUIT",
		fore, back, []() { gui::quit = true; }));
	gui::elements.push_back(new gui::button(20, 20, 110, 42, false, 10, "<BACK",
		fore, back, []() {
		int index = 0;
		for (; index < gui::elements.size() && gui::elements.data()[index]->text != "<BACK"; index++);
		for (int i = 1; i < index; i++)
			gui::elements[i]->visible = true;
		for (int i = index; i < index + 18; i++)
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
	gui::elements.push_back(new gui::label(50, 450, 1000, false, "Toggle/Move", fore, 2));
	gui::elements.push_back(new gui::keybind_button(550, 440, false, &keybinds::toggle, fore, back));
}

static void draw(int c, int x, int y, int size, glm::vec4 colour) {
	std::vector<rect>* tex;
	if (c >= 'a' && c <= 'z')
		tex = &gui::font[c - 'a' + 'A'];
	else if (c > 127) {
		switch (c) {
		case GLFW_KEY_RIGHT:		tex = &gui::font[1];	break;
		case GLFW_KEY_LEFT:			tex = &gui::font[2];	break;
		case GLFW_KEY_DOWN:			tex = &gui::font[3];	break;
		case GLFW_KEY_UP:			tex = &gui::font[4];	break;
		case GLFW_KEY_LEFT_SHIFT:	tex = &gui::font[5];	break;
		case GLFW_KEY_RIGHT_SHIFT:	tex = &gui::font[6];	break;
		case GLFW_KEY_CAPS_LOCK:	tex = &gui::font[6];	break;
		case GLFW_KEY_BACKSPACE:	tex = &gui::font['\b'];	break;
		case GLFW_KEY_TAB:			tex = &gui::font['\t'];	break;
		case GLFW_KEY_ENTER:		tex = &gui::font['\r'];	break;
		default:					tex = &gui::font[0];	break;
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
			1, glm::value_ptr(colour));
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::rect_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_SCISSOR_TEST);
	}
}

static void render_text(std::string text, int x, int y, int width, int size, glm::vec4 colour) {
	if (width == 0)
		return;
	GLint gl_x = x, gl_y = 720 - y - size * 11;
	width = (width + size * 2) / (size * 9);
	GLint xpos = 0, ypos = 0;
	size_t prev = 0;
	for (size_t cur = 0;
		(cur = text.find_first_of(" /\\\f\n\r\t\v-()[]{}", prev)) != std::string::npos;
		prev = cur + 1) {
		std::string token = text.substr(prev, cur - prev);
		if ((token.length() <= width && xpos + token.length() > width)
			|| xpos >= width) {
			xpos = 0;
			ypos++;
		}
		for (int j = 0; j < token.length(); j++) {
			draw(token.data()[j], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			if (++xpos >= width) {
				xpos = 0;
				ypos++;
			}
		}
		if (text.data()[cur] == '\r' || text.data()[cur] == '\n') {
			xpos = 0;
			ypos++;
		}
		else {
			draw(text.data()[cur], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			xpos++;
		}
	}
	if (prev < text.length()) {
		std::string token = text.substr(prev);
		if (token.length() <= width && xpos + token.length() > width
			|| xpos >= width) {
			xpos = 0;
			ypos++;
		}
		for (int j = 0; j < token.length(); j++) {
			draw(token.data()[j], gl_x + xpos * size * 9, gl_y - ypos * size * 16, size, colour);
			if (++xpos >= width) {
				xpos = 0;
				ypos++;
			}
		}
	}
}

gui::element::element(int x, int y, int width, bool visible,
	std::string text, glm::vec4 foreground) :
	x(x), y(y), width(width), text(text), visible(visible), foreground(foreground) {}

gui::panel::panel(int x, int y, int width, int height, bool visible,
	glm::vec4 colour, GLfloat noise) :
	element(x, y, width, visible, "", colour), height(height), noise(noise), offset(rand() / ((RAND_MAX + 1) / 256)) { }

void gui::panel::render(void) {
	if (!visible)
		return;
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 720 - y - height, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
		1, glm::value_ptr(foreground));
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), noise);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 3.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::rect_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
}

gui::label::label(int x, int y, int width, bool visible,
	std::string text, glm::vec4 foreground, int size) :
	element(x, y, width, visible, text, foreground), size(size) {}

void gui::label::render(void) {
	if (visible)
		render_text(text, x, y, width, size, foreground);
}

gui::button::button(int x, int y, int width, int height,
	bool visible, int margin, std::string text,
	glm::vec4 foreground, glm::vec4 background, void (*event)(void)) :
	element(x, y, width, visible, text, foreground), height(height), margin(margin),
	state(0), inactive(background), event(event) {
	hover = glm::vec4(background.r * 1.2f, background.g * 1.2f, background.b * 1.2f, background.a);
	click = glm::vec4(background.r * 1.4f, background.g * 1.4f, background.b * 1.4f, background.a);
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
	glm::vec4 colour;
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
		1, glm::value_ptr(colour));
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::rect_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	render_text(text, x + margin, y + margin, width - 2 * margin, 2, foreground);
}

gui::keybind_button::keybind_button(int x, int y, bool visible, int* key,
	glm::vec4 foreground, glm::vec4 background) :
	button(x, y, 34, 42, visible, 10, "", foreground, background, NULL),
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
			glfwGetCursorPos(window, &_x, &_y);
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 2;
		}
		break;
	case enum_event::KEY:
		if (b == GLFW_PRESS && state == 2) {
			if (a != GLFW_KEY_ESCAPE)
				*key = a;
			double _x = 0, _y = 0;
			glfwGetCursorPos(window, &_x, &_y);
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 1;
		}
		break;
	}
}

void gui::keybind_button::render(void) {
	if (!visible)
		return;
	button::render();
	draw(*key, x + margin, 698 - (y + margin), 2, foreground);
}

gui::textbox::textbox(int x, int y, int width, int height,
	bool visible, int margin, std::string text,
	glm::vec4 foreground, glm::vec4 background) :
	element(x, y, width, visible, text, foreground), height(height), margin(margin),
	state(0), inactive(background) {
	active = glm::vec4(background.r * 1.4f, background.g * 1.4f, background.b * 1.4f, background.a);
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
			glfwGetCursorPos(window, &_x, &_y);
			state = _x < x || _x > x + width || _y < y || _y > y + height ? 0 : 2;
		}
		break;
	case enum_event::TEXT:
		if (state != 2)
			break;
		if (a <= 0x7f)
			text += (char)a;
		else {
			// this is deprecated but I don't care
			unsigned codepoint = (unsigned)a;
			text += std::wstring_convert<std::codecvt_utf8<unsigned>, unsigned>().to_bytes(&codepoint, &codepoint + 1);
		}
		break;
	case enum_event::KEY:
		if (b == GLFW_RELEASE || state != 2)
			break;
		if (c & GLFW_MOD_CONTROL) {
			switch (a) {
			case GLFW_KEY_C:
				glfwSetClipboardString(NULL, text.c_str());
				break;
			case GLFW_KEY_V:
				if (const char* str = glfwGetClipboardString(NULL))
					text += str;
				break;
			case GLFW_KEY_X:
				glfwSetClipboardString(NULL, text.c_str());
				text.clear();
				break;
			case GLFW_KEY_BACKSPACE:
				size_t index = text.find_last_of("\\/ \t\r\n");
				text.erase(index == std::string::npos ? 0 : index);
				break;
			}
		}
		else if (a == GLFW_KEY_BACKSPACE && !text.empty())
			text.pop_back();
		break;
	}
}

void gui::textbox::render(void) {
	if (!visible)
		return;
	glm::vec4 colour = state > 0 ? active : inactive;
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, 720 - y - height, width, height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
		1, glm::value_ptr(colour));
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
	glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), 0.0f);
	glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
	glUseProgram(res::shaders::rectangle);
	glBindVertexArray(res::rect_vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);
	render_text(text, x + margin, y + margin, width - 2 * margin, 2, foreground);
}
