#ifndef _HEAD_HPP_
#define _HEAD_HPP_

// Yes, I used 3 different data structures for this
// Why did I use them? I don't even know anymore
#include <deque>
#include <list>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define PX_SIZE 2
#define PI 3.14159265359

struct rect {
	GLint x, y;
	GLsizei width, height;
	glm::vec4 colour;
	GLfloat noise;
};

struct box {
	double x[4];
	double y[4];
};

class object {
public:
	static object* selected;

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
		SPDC_CRYSTAL, MOVING_CRYSTAL, SPLITTER, FIXED_SPLITTER,
		BOMB, SENSOR,
		NONE
	};
	int x, y, width, height, offset;
	double _x, _y;
	int x1, y1, x2, y2;
	bool toggle, moving;
	enum_orientation orientation;
	enum_type type;
	std::vector<struct rect>* texture;
	std::vector<struct rect>* other_tex;
	std::vector<struct box>* hitbox;
	
	object(double x, double y, int width, int height, enum_orientation direction, enum_type type,
		std::vector<struct rect>* tex, std::vector<struct box>* hbox);
	
	virtual void render(void);
	virtual void tick(int tps);
	inline virtual bool on_screen(void) const { return x < 1280 / PX_SIZE && x + width > 0 && y < 720 / PX_SIZE && y + height > 0; };
	double angle(void) const;
	inline int midx(void) const { return x + (width / 2); }
	inline int midy(void) const { return y + (height / 2); }
};

class node;

class photon : public object {
public:
	static std::list<photon> photons;
	static std::list<node> nodes;
	static std::vector<node*> removing;

	// This program uses approximations to make level design ever so slightly easier
	// The fact that arctan 1/3 + arctan 1/2 = pi/4 is pretty nice
	// Note: changing this system will require modifications to the rest of the code, good luck
	enum class enum_direction {
		E, E_ATAN1_3, E_ATAN1_2, NE, E_ATAN2, E_ATAN3,
		N, N_ATAN1_3, N_ATAN1_2, NW, N_ATAN2, N_ATAN3,
		W, W_ATAN1_3, W_ATAN1_2, SW, W_ATAN2, W_ATAN3,
		S, S_ATAN1_3, S_ATAN1_2, SE, S_ATAN2, S_ATAN3,
		NONE
	};
	double _tick;
	int dc;
	object* medium;
	node* parent;
	enum_direction direction;
	std::deque<int> immune;
	std::deque<object*> interacted;
	
	photon(std::vector<struct rect>* texture, double x, double y, enum_direction dir, int dc, node* parent);

	void render(void);
	inline bool on_screen(void) const { return _x < 1280.0 / PX_SIZE && _x + width > 0.0 && _y < 720.0 / PX_SIZE && _y + height > 0.0; }
	void pre_tick(int tps);
	void tick(int tps, double len, std::list<photon>::iterator* iter);
	double heading(void) const;
	inline double vel(int tps) const { return 160.0 / tps / (medium ? 1.5 : 1.0); }
	inline double velx(int tps) const { return direction == enum_direction::NONE ? 0.0 : vel(tps) * cos(heading()); }
	inline double vely(int tps) const { return direction == enum_direction::NONE ? 0.0 : vel(tps) * sin(heading()); }
	void destroy(void);
private:
	object* interacting;
	box i_hbox;
	int i_line;
};

class node {
public:
	enum class enum_node {
		SUPERPOS, SPDC
	};

	node* parent;
	enum_node type;
	std::vector<node*> children;
	std::vector<photon*> items;

	node(node* parent, enum_node type);

	static void move(node* victim, node* const target);
	static void move(photon* victim, node* const target);

	inline node* add(enum_node type) { photon::nodes.push_back(node(this, type)); return &photon::nodes.back(); }
	void destroy(void);
};

namespace res {
	extern GLuint rect_vao;
	extern std::vector<object> objects;
	namespace shaders {
		extern GLuint rectangle;
		extern const char* vertex;
		extern const char* fragment;
		void init(void);
	}
	namespace loader {
		extern std::vector<std::vector<box>> hitboxes;
		extern std::vector<std::vector<rect>> textures;
		void init(void);
	}
	void init_vao(void);
}

#endif // _HEAD_HPP_
