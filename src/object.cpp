// Maybe using smart pointers would have been a good idea :)
// Unfortunately I was too lazy to use them

#include <algorithm>
#include <cfloat>
#include <cstdlib>

#include <glad/glad.h>

#include "head.hpp"

const rect rect::background = { 0, 0, 640, 360, vec(0.08f, 0.08f, 0.08f, 1.0f), 5.0f };

inline static GLint _round(double x) {
	return (GLint)std::round(x);
}

// object

object::object(void) :
	x(0), y(0), _x(0.0), _y(0.0), width(0), height(0),
	x1(0), y1(0), x2(0), y2(0),
	offset(0), toggle(false), moving(false), linked(NULL),
	orientation(enum_orientation::NONE), type(enum_type::NONE),
	texture(NULL), hitbox(NULL), data(0) {}

object::object(double x, double y, int width, int height, enum_orientation orientation, enum_type type,
	std::vector<struct rect>* tex, std::vector<struct box>* hbox, int data) :
	x((int)x), y((int)y), _x(x), _y(y), width(width), height(height),
	x1((int)x), y1((int)y), x2((int)x), y2((int)y),
	offset(rand() / ((RAND_MAX + 1) / 256)), toggle(false), moving(false),
	linked(NULL), orientation(orientation), type(type),
	texture(tex), hitbox(hbox), data(data) {}

object* object::selected = NULL;
object* object::previous = NULL;
std::vector<object*> object::invalidated;
bool object::invalidate_all;
std::list<group> object::groups;

void object::render(int layer) const {
	if (!on_screen())
		return;
	if (!texture) {
		if (layer)
			return;
		const rect& quad = rect::background;
		glEnable(GL_SCISSOR_TEST);
		glScissor(
			(x + quad.x) * PX_SIZE, (y + quad.y) * PX_SIZE,
			quad.width * PX_SIZE, quad.height * PX_SIZE);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
			1, quad.colour.ptr());
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), quad.noise);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
			(GLfloat)offset, (GLfloat)offset);
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::rect_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_SCISSOR_TEST);
		return;
	}
	if (layer == -1) {
		glEnable(GL_SCISSOR_TEST);
		GLint __x = x * PX_SIZE;
		GLint __y = y * PX_SIZE;
		GLsizei w = width * PX_SIZE;
		GLsizei h = height * PX_SIZE;
		if (__x < 0) {
			w += __x;
			__x = 0;
		}
		if (__y < 0) {
			h += __y;
			__y = 0;
		}
		if (w > 0 && h > 0 && x < 1280 && y < 720) {
			glScissor(x * PX_SIZE, y * PX_SIZE, width * PX_SIZE, height * PX_SIZE);
			glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"), 1,
				rect::background.colour.ptr());
			glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"),
				rect::background.noise);
			glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
			glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
				(GLfloat)res::objects.data()[0].offset,
				(GLfloat)res::objects.data()[0].offset);
			glUseProgram(res::shaders::rectangle);
			glBindVertexArray(res::rect_vao);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			glDisable(GL_SCISSOR_TEST);
		}
	}
	std::vector<struct rect>* tex = texture;
	switch (type) {
	case enum_type::DOOR:
	case enum_type::MIRROR_DOOR:
		tex += (int)toggle;
		break;
	case enum_type::MIRROR:
		tex += (int)orientation % 8;
		break;
	case enum_type::DIAGONAL_MIRROR:
	case enum_type::SPLITTER:
		tex += ((int)orientation - 2) % 8 == 0 ? 0 : 1;
		break;
	case enum_type::GLASS_BLOCK:
		tex += (int)orientation % 8 / 2;
		break;
	case enum_type::BOMB:
		tex += (int)gamestate::hardcore;
		break;
	default:
		break;
	}
	for (int i = 0; i < tex->size(); i++) {
		const rect& quad = tex->data()[i];
		if (layer >= 0) {
			if (quad.layer < layer)
				continue;
			if (quad.layer > layer)
				return;
		}
		GLint __x = _round((_x + quad.x) * PX_SIZE);
		GLint __y = _round((_y + quad.y) * PX_SIZE);
		GLsizei w = quad.width * PX_SIZE;
		GLsizei h = quad.height * PX_SIZE;
		if (__x < 0) {
			w += __x;
			__x = 0;
		}
		if (__y < 0) {
			h += __y;
			__y = 0;
		}
		if (w <= 0 || h <= 0 || __x > 1280 || __y > 720)
			continue;
		glEnable(GL_SCISSOR_TEST);
		glScissor(__x, __y, w, h);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if (type == enum_type::SENSOR && quad.colour == vec(0.3f, 0.9f, 0.6f, 1.0f)) {
			vec colour = data ? vec(0.3f, 0.9f, 0.6f, 1.0f) : vec(0.8f, 0.9f, 0.2f, 1.0f);
			glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
				1, colour.ptr());
		}
		else
			glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
				1, quad.colour.ptr());
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), quad.noise);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
			(GLfloat)(offset - _x), (GLfloat)(offset - _y));
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::rect_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
	}
}

void object::border(bool clear) const {
	std::vector<rect> r;
	if (this == object::selected || clear) {
		r.push_back({ _round((_x - 4) * PX_SIZE), _round((_y - 4) * PX_SIZE), (width + 7) * PX_SIZE, PX_SIZE });
		r.push_back({ _round((_x - 4) * PX_SIZE), _round((_y - 3) * PX_SIZE), PX_SIZE, (height + 6)* PX_SIZE });
		r.push_back({ _round((_x - 4) * PX_SIZE), _round((_y + height + 3) * PX_SIZE), (width + 8)* PX_SIZE, PX_SIZE });
		r.push_back({ _round((_x + width + 3) * PX_SIZE), _round((_y - 4) * PX_SIZE), PX_SIZE, (height + 7)* PX_SIZE });
	}
	else {
		int __x = _round((_x - 3) * PX_SIZE);
		int __y = _round((_y - 4) * PX_SIZE);
		int xmax = _round((_x + width + 2) * PX_SIZE);
		int ymax = _round((_y + height + 2) * PX_SIZE);
		for (; __x <= xmax; __x += 7 * PX_SIZE) {
			int w = 5 * PX_SIZE;
			while (__x + w > xmax + PX_SIZE)
				w--;
			r.push_back({ __x, __y, w, PX_SIZE });
			r.push_back({ __x, ymax + PX_SIZE, w, PX_SIZE });
		}
		__x = _round((_x - 4) * PX_SIZE);
		__y += PX_SIZE;
		for (; __y <= ymax; __y += 7 * PX_SIZE) {
			int h = 5 * PX_SIZE;
			while (__y + h > ymax + PX_SIZE)
				h--;
			r.push_back({ __x, __y, PX_SIZE, h });
			r.push_back({ xmax + PX_SIZE, __y, PX_SIZE, h });
		}
	}
	vec col = vec(1.0f, 1.0f, 1.0f, this == object::selected ? 0.6f : 0.3f);
	if (clear)
		col = rect::background.colour;
	for (int i = 0; i < r.size(); i++) {
		GLint __x = r.data()[i].x;
		GLint __y = r.data()[i].y;
		GLsizei w = r.data()[i].width;
		GLsizei h = r.data()[i].height;
		if (__x < 0) {
			w += __x;
			__x = 0;
		}
		if (__y < 0) {
			h += __y;
			__y = 0;
		}
		if (w <= 0 || h <= 0 || __x > 1280 || __y > 720)
			continue;
		glEnable(GL_SCISSOR_TEST);
		glScissor(__x, __y, w, h);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"), 1, col.ptr());
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"),
			clear ? rect::background.noise : 0.0f);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"),
			(GLfloat)res::objects.data()[0].offset, (GLfloat)res::objects.data()[0].offset);
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::rect_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
	}
}

void object::tick(int tps) {
	int dest_x = toggle ? x2 : x1;
	int dest_y = toggle ? y2 : y1;
	if (x == dest_x && dest_y) {
		_x = dest_x;
		_y = dest_y;
		moving = false;
		return;
	}
	object::invalidate_all = true;
	double angle = std::atan2(2 * dest_y - y2 - y1, 2 * dest_x - x2 - x1);
	double vel = 3.0 * distance(x, y, dest_x, dest_y) / tps;
	if (vel > 120.0 / tps)
		vel = 120.0 / tps;
	else if (vel < 20.0 / tps)
		vel = 20.0 / tps;
	if (distance(x, y, dest_x, dest_y) < vel)
		vel = distance(x, y, dest_x, dest_y);
	_x += vel * std::cos(angle);
	_y += vel * std::sin(angle);
	x = (int)std::round(_x);
	y = (int)std::round(_y);
}

double object::angle() const {
	if (orientation == enum_orientation::NONE)
		return 0.0;
	int tmp = (int)orientation / 4 + 1;
	tmp = tmp == 4 ? 0 : tmp;
	double angle = PI * (double)tmp / 2.0;
	switch ((int)orientation % 4) {
	case 0: 							break;
	case 1: angle -= 1.0 * PI / 8.0;	break;
	case 2: angle -= 2.0 * PI / 8.0;	break;
	case 3: angle -= 3.0 * PI / 8.0;	break;
	}
	return angle;
}

// group

void group::add(object* obj) {
	members.push_back(obj);
	obj->linked = this;
}

// photon

photon::photon(std::vector<struct rect>* tex, double x, double y, enum_direction dir, int dc, int split, node* parent) :
	object(x, y, 4, 4, enum_orientation::NONE, enum_type::NONE, NULL, NULL, 0),
	_tick(0.0), medium(NULL), direction(dir), dc(dc), split(split), parent(parent),
	interacting(NULL), i_hbox(), i_line(0) {
	texture = tex;
	id = count++;
}

int photon::count = 0;

std::list<photon> photon::photons;
std::list<node> photon::nodes;
std::unordered_set<node*> photon::deleting;

void photon::render(void) const {
	if (texture == NULL || !on_screen() || direction == enum_direction::NONE)
		return;
	for (int i = 0; i < texture->size(); i++) {
		rect quad = texture->data()[i];
		double __x = (_x + quad.x) * PX_SIZE;
		double __y = (_y + quad.y) * PX_SIZE;
		double w = quad.width * PX_SIZE;
		double h = quad.height * PX_SIZE;
		if (__x < 0) {
			w += __x;
			__x = 0;
		}
		if (__y < 0) {
			h += __y;
			__y = 0;
		}
		if ((GLsizei)w <= 0 || (GLsizei)h <= 0 || _round(__x) > 1280 || _round(__y) > 720)
			continue;
		glEnable(GL_SCISSOR_TEST);
		glScissor(_round(__x), _round(__y), (GLsizei)w, (GLsizei)h);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
			1, quad.colour.ptr()); glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
		glUniform1f(glGetUniformLocation(res::shaders::rectangle, "pxsize"), (GLfloat)PX_SIZE);
		glUniform2f(glGetUniformLocation(res::shaders::rectangle, "offset"), 0.0f, 0.0f);
		glUseProgram(res::shaders::rectangle);
		glBindVertexArray(res::rect_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glDisable(GL_SCISSOR_TEST);
	}
}

void photon::pre_tick(int tps) {
	object* obj = NULL;
	box hitbox;
	double dist = 0.0, best = 1.5;
	int line = 0;
	for (int i = 0; i < res::objects.size(); i++) {
		std::deque<object*>::iterator iter;
		if (res::objects.data()[i].type == enum_type::NONE)
			continue;
		else if (interacted.size() != 0
			&& (iter = std::find(interacted.begin(), interacted.end(), &res::objects.data()[i])) != interacted.end())
			continue;
		const object& tmp = res::objects.data()[i];
		if (tmp.hitbox == NULL) {
			box hbox = {
				{-0.5, -0.5, tmp.width + 0.5, tmp.width + 0.5},
				{-0.5, tmp.height + 0.5, tmp.height + 0.5, -0.5}
			};
			for (int p = 0, q = 1; p < 4; p++, q = (p + 1 == 4 ? 0 : p + 1)) {
				if ((dist = intersect(_x, _y, _x + velx(tps), _y + vely(tps),
					tmp.x + hbox.x[p], tmp.y + hbox.y[p],
					tmp.x + hbox.x[q], tmp.y + hbox.y[q]))
					< best) {
					obj = &res::objects.data()[i];
					hitbox = hbox;
					best = dist;
					line = p;
				}
			}
		}
		else {
			std::vector<box>* rotated = tmp.hitbox;
			switch (tmp.type) {
			case enum_type::MIRROR:
				rotated += (int)orientation % 8;
				break;
			case enum_type::DIAGONAL_MIRROR:
			case enum_type::SPLITTER:
				rotated += ((int)orientation - 2) % 8 == 0 ? 0 : 1;
				break;
			case enum_type::GLASS_BLOCK:
				rotated += (int)orientation % 8 / 2;
				break;
			default:
				break;
			}
			for (int j = 0; j < tmp.hitbox->size(); j++) {
				const box& hbox = tmp.hitbox->data()[j];
				for (int p = 0, q = 1; p < 4; p++, q = (p + 1 == 4 ? 0 : p + 1)) {
					if (hbox.x[p] == -DBL_MAX && hbox.y[p] == -DBL_MAX)
						break;
					if (hbox.x[q] == -DBL_MAX && hbox.y[q] == -DBL_MAX)
						q = 0;
					if ((dist = intersect(_x, _y, _x + velx(tps), _y + vely(tps),
						tmp.x + hbox.x[p], tmp.y + hbox.y[p],
						tmp.x + hbox.x[q], tmp.y + hbox.y[q]))
						< best) {
						obj = &res::objects.data()[i];
						hitbox = hbox;
						best = dist;
						line = p;
					}
				}
			}
		}
	}
	for (int i = 0; i < immune.size(); immune[i++]--);
	while (!immune.empty() && immune.front() <= 0) {
		immune.pop_front();
		interacted.pop_front();
	}
	interacting = obj;
	if (interacting) {
		i_hbox = hitbox;
		i_line = line;
	}
	_tick = obj ? best : 0.0;
}

void photon::tick(int tps, double len, std::list<photon>::iterator* iter) {
	_x += velx(tps) * len;
	_y += vely(tps) * len;
	if (obj_dist(_x, _y, res::objects.data()[0]) > 20.0)
		direction = enum_direction::NONE;
	if (interacting) {
		_tick -= len;
		if (_tick < 0.01) {
			if (interact(this, len, interacting, &i_hbox, i_line, tps, iter))
				return;
			int duration = tps < 32 ? 1 : 2;
			std::deque<object*>::iterator iter;
			if ((iter = std::find(interacted.begin(), interacted.end(), interacting)) != interacted.end()) {
				if (immune[iter - interacted.begin()] < duration)
					immune[iter - interacted.begin()] = duration;
			}
			else {
				interacted.push_back(interacting);
				immune.push_back(duration);
			}
			_tick = 0.0;
			interacting = NULL;
		}
	}
	++(*iter);
}

double photon::heading() const {
	if (direction == enum_direction::NONE)
		return 0.0;
	double angle = PI * (double)((int)direction / 6) / 2.0;
	switch ((int)direction % 6) {
	case 0: 							break;
	case 1: angle += std::atan2(1.0, 3.0);	break;
	case 2: angle += std::atan2(1.0, 2.0);	break;
	case 3: angle += PI / 4.0;			break;
	case 4: angle += std::atan(2.0);			break;
	case 5: angle += std::atan(3.0);			break;
	}
	return angle;
}

void photon::destroy(void) {
	if (!parent)
		return;
	std::vector<photon*>::iterator it =
		std::remove(parent->items.begin(), parent->items.end(), this);
	parent->items.erase(it, parent->items.end());
}

// node

node::node(node* parent, enum_node type) : parent(parent), type(type) {}

void node::move(node* victim, node* const target) {
	if (victim == target)
		return;
	if (victim->parent) {
		std::vector<node*>::iterator it = std::remove(victim->parent->children.begin(), victim->parent->children.end(), victim);
		victim->parent->children.erase(it, victim->parent->children.end());
	}
	if (target) {
		if (target->type == victim->type) {
			for (int i = 0; i < victim->children.size(); i++)
				victim->children.data()[i]->parent = target;
			target->children.insert(target->children.end(), victim->children.begin(), victim->children.end());
			for (int i = 0; i < victim->items.size(); i++)
				victim->items.data()[i]->parent = target;
			target->items.insert(target->items.end(), victim->items.begin(), victim->items.end());
			victim->destroy();
			photon::nodes.remove_if([victim](const node& other) { return &other == victim; });
		}
		else {
			victim->parent = target;
			target->children.push_back(victim);
		}
	}
	else
		victim->parent = target;
}

void node::move(photon* victim, node* const target) {
	if (victim->parent) {
		std::vector<photon*>::iterator it = std::remove_if(
			victim->parent->items.begin(), victim->parent->items.end(),
			[victim](photon* other) { return other == victim; });
		victim->parent->items.erase(it, victim->parent->items.end());
	}
	victim->parent = target;
	if (target)
		target->items.push_back(victim);
}

node* node::add(enum_node type) {
	photon::nodes.push_back(node(this, type));
	node* n = &photon::nodes.back();
	children.push_back(n);
	return n;
}

void node::clear(void) {
	for (std::list<photon>::iterator it = photon::photons.begin(); it != photon::photons.end();) {
		photon* p = &*it;
		if (std::any_of(items.begin(), items.end(),
			[p](photon* other) { return other == p; }))
			it = photon::photons.erase(it);
		else
			++it;
	}
	for (std::list<node>::iterator it = photon::nodes.begin(); it != photon::nodes.end();) {
		node* n = &*it;
		if (std::any_of(children.begin(), children.end(),
			[n](node* other) { return other == n; }))
			it = photon::nodes.erase(it);
		else
			++it;
	}
}

void node::destroy(void) {
	if (!parent)
		return;
	std::vector<node*>::iterator it =
		std::remove(parent->children.begin(), parent->children.end(), this);
	parent->children.erase(it, parent->children.end());
}
