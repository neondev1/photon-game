#ifndef _GUI_HPP_
#define _GUI_HPP_

#include <string>

#include "head.hpp"

namespace gui {
	enum class enum_event { MOVE, CLICK, TEXT, KEY };

	class element {
	public:
		int x, y, width;
		std::string text;
		bool visible;
		glm::vec4 foreground;

		element(int x, int y, int width, bool visible,
			std::string text, glm::vec4 foreground);

		virtual void handler(enum_event, int, int, int) = 0;
		virtual void render(void) = 0;
	};

	class panel : public element {
	public:
		int height, offset;
		GLfloat noise;

		panel(int x, int y, int width, int height, bool visible,
			glm::vec4 colour, GLfloat noise);
	
		inline void handler(enum_event, int, int, int) {}
		void render(void);
	};

	class label : public element {
	public:
		int size;
		
		label(int x, int y, int width, bool visible,
			std::string text, glm::vec4 foreground, int size);

		inline void handler(enum_event, int, int, int) {}
		void render(void);
	};

	class button : public element {
	public:
		int height, margin;
		int state;
		glm::vec4 inactive, hover, click;
		void (*event)(void);

		button(int x, int y, int width, int height,
			bool visible, int margin, std::string text,
			glm::vec4 foreground, glm::vec4 background, void (*event)(void));

		virtual void handler(enum_event type, int a, int b, int c);
		void render(void);
	};

	class keybind_button : public button {
	public:
		int* key;

		keybind_button(int x, int y, bool visible, int* key,
			glm::vec4 foreground, glm::vec4 background);

		void handler(enum_event type, int a, int b, int c);
		void render(void);
	};

	class textbox : public element {
	public:
		int height, margin;
		int state;
		glm::vec4 inactive, active;

		textbox(int x, int y, int width, int height,
			bool visible, int margin, std::string text,
			glm::vec4 foreground, glm::vec4 background);

		void handler(enum_event type, int a, int b, int c);
		void render(void);
	};

	void load_font(void);
	void load_gui(void);

	extern bool menu;
	extern bool started;
	extern bool quit;
	extern std::vector<element*> elements;
	extern std::vector<rect> font[128];
}

#endif // _GUI_HPP_
