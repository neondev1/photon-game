#ifndef _HEAD_HPP_
#define _HEAD_HPP_

#include <deque>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cmath>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define PX_SIZE 2
#define PI 3.14159265359

struct vec {
	union { float x, r, s; };
	union { float y, g, t; };
	union { float z, b, p; };
	union { float w, a, q; };

	inline vec(void) : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	inline vec(float x, float y, float z, float w) :
		x(x), y(y), z(z), w(w) {}

	inline float const* ptr(void) const { return &x; }
};

inline bool operator==(const vec& left, const vec& right) {
	return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

struct rect {
	GLint x, y;
	GLsizei width, height;
	vec colour;
	GLfloat noise;
	int layer;
};

struct box {
	double x[4];
	double y[4];
};

class group;

class object {
public:
	static const vec default_colour;
	static const GLfloat default_noise;

	static object* selected;
	static object* previous;
	static std::list<group> groups;
	static bool invalidate_all;
	static std::unordered_set<object*> invalidated;
	static std::unordered_map<unsigned, std::vector<rect>> temp_tex;

	enum class enum_orientation {
		N, NNW, NW, NWW,
		W, SWW, SW, SSW,
		S, SSE, SE, SEE,
		E, NEE, NE, NNE,
		NONE
	};
	enum class enum_type {
		WALL, DOOR, MOVING_WALL,
		MIRROR, DIAGONAL_MIRROR, MIRROR_BLOCK, MIRROR_DOOR,
		GLASS_BLOCK, FIXED_BLOCK, MOVING_BLOCK, PRISM,
		SPDC_CRYSTAL, MOVING_CRYSTAL, SPLITTER,
		BOMB, SENSOR,
		NONE
	};
	int x, y, width, height, offset;
	double _x, _y;
	int x1, y1, x2, y2;
	bool toggle, moving;
	group* linked;
	enum_orientation orientation;
	enum_type type;
	std::vector<rect>* texture;
	std::vector<box>* hitbox;
	int data;
	bool randomize;

	object(void);
	object(double x, double y, int width, int height, enum_orientation direction, enum_type type,
		std::vector<rect>* tex, std::vector<box>* hbox, int data, bool randomize);
	
	static bool overlapping(const object& obj1, const object& obj2);

	void render(int layer) const;
	void border(bool clear = false) const;
	void tick(int tps);
	inline virtual bool on_screen(void) const { return x < 1280 / PX_SIZE && x + width > 0 && y < 720 / PX_SIZE && y + height > 0; };
	double angle(void) const;
	inline int midx(void) const { return x + (width / 2); }
	inline int midy(void) const { return y + (height / 2); }
};

class group {
public:
	std::vector<object*> members;

	void add(object* obj);
};

class node;

class photon: public object {
public:
	static int count; // for debugging

	static std::list<photon> photons;
	static std::list<node> nodes;
	static std::unordered_set<node*> deleting;

	// This program uses approximations to make level design ever so slightly easier
	// The fact that arctan 1/3 + arctan 1/2 = pi/4 is pretty nice
	// Note: changing this system will require modifications to the rest of the code, have fun
	enum class enum_direction {
		E, E_ATAN1_3, E_ATAN1_2, NE, E_ATAN2, E_ATAN3,
		N, N_ATAN1_3, N_ATAN1_2, NW, N_ATAN2, N_ATAN3,
		W, W_ATAN1_3, W_ATAN1_2, SW, W_ATAN2, W_ATAN3,
		S, S_ATAN1_3, S_ATAN1_2, SE, S_ATAN2, S_ATAN3,
		NONE
	};

	int id; // for debugging

	double _tick;
	int dc, split;
	object* medium;
	node* parent;
	enum_direction direction;
	std::deque<int> immune;
	std::deque<object*> interacted;
	
	photon(std::vector<rect>* texture, double x, double y, enum_direction dir, int dc, int split, node* parent);

	void render(void) const;
	inline bool on_screen(void) const { return _x < 1280.0 / PX_SIZE && _x + width > 0.0 && _y < 720.0 / PX_SIZE && _y + height > 0.0; }
	void pre_tick(int tps);
	void tick(int tps, double len, std::list<photon>::iterator* iter);
	double heading(void) const;
	inline double vel(int tps) const { return 160.0 / tps / (medium ? 1.5 : 1.0); }
	inline double velx(int tps) const { return direction == enum_direction::NONE ? 0.0 : vel(tps) * std::cos(heading()); }
	inline double vely(int tps) const { return direction == enum_direction::NONE ? 0.0 : vel(tps) * std::sin(heading()); }
	void destroy(void);
private:
	object* interacting;
	box i_hbox;
	int i_line;
};

class node {
public:
	enum class enum_node { SUPERPOS, SPDC };

	node* parent;
	enum_node type;
	std::vector<node*> children;
	std::vector<photon*> items;

	node(node* parent, enum_node type);

	static void move(node* victim, node* const target);
	static void move(photon* victim, node* const target);

	node* add(enum_node type);
	void clear(void);
	void destroy(void);
};

namespace game {
	extern bool started;
	extern bool hint;
	extern long long frame;
	extern bool hardcore;
	extern int level;
	extern int failures;
	extern int sensors;
	extern int activated;
	extern double time;
	extern std::string save;
	extern std::string name;
	extern bool custom;
}

namespace res {
	extern GLuint rect_vao;
	extern std::vector<object> objects;
	namespace shaders {
		extern GLuint rectangle;
		extern const char* vertex;
		extern const char* fragment;
		void load(void);
	}
	namespace loader {
		class level {
		public:
			std::string hint;
			bool hint_seen;
			std::vector<object> objects;

			inline level(void) : hint_seen(false) {}
		};
		extern std::vector<rect> textures[32];
		extern std::vector<box> hitboxes[32];
		extern std::vector<level> levels;
		void load_tex(void);
		void load_hbx(void);
		void load_default(void);
		bool load_from_file(std::string path);
		void load_level(int level, bool randomize_orientation);
		std::vector<rect>* get_tex(object::enum_type type);
		std::vector<box>* get_hbx(object::enum_type type);
		const rect& background(int level = game::level);
		void set_background(int level, const vec colour, float noise);
	}
	void load_vao(void);
}

namespace keybinds {
	extern int up, left, down, right;
	extern int ccw, cw, perp, toggle;
	extern int hint;
}

double intersect(double x1, double y1, double x2, double y2,
	double x3, double y3, double x4, double y4);
double obj_dist(double x, double y, const object& obj);
bool interact(photon* const p, double dist, object* const obj,
	box const* const hitbox, int line, int tps, std::list<photon>::iterator* iter);
void select(int key);
void render_bars(void);
void render_text(std::string text, int x, int y, int width, int size, bool cursor, size_t cursor_pos, vec colour);

unsigned mix32_rand(void);
unsigned mix32_rand(unsigned n);

inline double distance(double x1, double y1, double x2, double y2) {
	return std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

#endif // _HEAD_HPP_
