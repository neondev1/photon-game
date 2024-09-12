// Maybe using smart pointers would have been a good idea :)
// Unfortunately I was too lazy to use them

#include <algorithm>
#include <limits>
#include <cstdlib>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "head.hpp"

// object

object::object(void) :
	x(0), y(0), _x(0.0), _y(0.0), width(0), height(0),
	x1(0), y1(0), x2(0), y2(0),
	offset(0), toggle(false), moving(false),
	orientation(enum_orientation::NONE), type(enum_type::NONE),
	texture(NULL), other_tex(NULL), hitbox(NULL) {}

object::object(double x, double y, int width, int height, enum_orientation orientation, enum_type type,
	std::vector<struct rect>* tex, std::vector<struct box>* hbox) :
	x((int)x), y((int)y), _x(x), _y(y), width(width), height(height),
	x1((int)x), y1((int)y), x2((int)x), y2((int)y),
	offset(rand() / ((RAND_MAX + 1) / 256)), toggle(false), moving(false),
	orientation(orientation), type(type),
	texture(tex), other_tex(tex), hitbox(hbox) {}

object* object::selected = NULL;

void object::render(int layer) const {
	if (texture == NULL || !on_screen())
		return;
	std::vector<struct rect>* tex = toggle ? other_tex : texture;
	switch (type) {
	case enum_type::MIRROR:
		tex += (int)orientation % 8;
		break;
	case enum_type::DIAGONAL_MIRROR:
		tex += ((int)orientation - 2) % 8 == 0 ? 0 : 1;
		break;
	case enum_type::GLASS_BLOCK:
		tex += (int)orientation % 8 / 2;
		break;
	default:
		break;
	}
	for (int i = 0; i < tex->size(); i++) {
		rect& quad = tex->data()[i];
		if (quad.layer < layer)
			continue;
		if (quad.layer > layer)
			return;
		glEnable(GL_SCISSOR_TEST);
		glScissor(
			(GLint)round((_x + quad.x) * PX_SIZE), (GLint)round((_y + quad.y) * PX_SIZE),
			quad.width * PX_SIZE, quad.height * PX_SIZE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
			1, glm::value_ptr(quad.colour));
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

void object::border(void) const {
	if (this == object::selected) {
		rect r[4] = {
			{(GLint)round((_x - 4) * PX_SIZE), (GLint)round((_y - 4) * PX_SIZE), (width + 7) * PX_SIZE, PX_SIZE},
			{(GLint)round((_x - 4) * PX_SIZE), (GLint)round((_y - 3) * PX_SIZE), PX_SIZE, (height + 6) * PX_SIZE},
			{(GLint)round((_x - 4) * PX_SIZE), (GLint)round((_y + height + 3) * PX_SIZE), (width + 8) * PX_SIZE, PX_SIZE},
			{(GLint)round((_x + width + 3) * PX_SIZE), (GLint)round((_y - 4) * PX_SIZE), PX_SIZE, (height + 7) * PX_SIZE}
		};
		for (int i = 0; i < 4; i++) {
			double __x = r[i].x;
			double __y = r[i].y;
			double w = r[i].width;
			double h = r[i].height;
			if (__x < 0) {
				w += __x;
				__x = 0;
			}
			if (__y < 0) {
				h += __y;
				__y = 0;
			}
			if ((GLsizei)w <= 0 || (GLsizei)h <= 0
				|| (GLint)__x > 1280.0 || (GLint)__y > 720.0)
				continue;
			glEnable(GL_SCISSOR_TEST);
			glScissor((GLint)__x, (GLint)__y, (GLsizei)w, (GLsizei)h);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniform4f(glGetUniformLocation(res::shaders::rectangle, "colour"), 1.0f, 1.0f, 1.0f, 0.5f);
			glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
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
	double angle = atan2(2 * dest_y - y2 - y1, 2 * dest_x - x2 - x1);
	double vel = 3.0 * distance(x, y, dest_x, dest_y) / tps;
	if (vel > 120.0 / tps)
		vel = 120.0 / tps;
	else if (vel < 20.0 / tps)
		vel = 20.0 / tps;
	if (distance(x, y, dest_x, dest_y) < vel)
		vel = distance(x, y, dest_x, dest_y);
	_x += vel * cos(angle);
	_y += vel * sin(angle);
	x = (int)round(_x);
	y = (int)round(_y);
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

// photon

photon::photon(std::vector<struct rect>* tex, double x, double y, enum_direction dir, int dc, node* parent) :
	object(x, y, 4, 4, enum_orientation::NONE, enum_type::NONE, NULL, NULL),
	_tick(0.0), medium(NULL), direction(dir), dc(dc), parent(parent),
	interacting(NULL), i_hbox(), i_line(0) {
	texture = tex;
}

std::list<photon> photon::photons;
std::list<node> photon::nodes;
std::vector<node*> photon::removing;

void photon::render(void) const {
	if (texture == NULL || !on_screen() || direction == enum_direction::NONE)
		return;
	for (int i = 0; i < texture->size(); i++) {
		rect quad = texture->data()[i];
		glEnable(GL_SCISSOR_TEST);
		double __x = (_x + (double)quad.x) * PX_SIZE;
		double __y = (_y + (double)quad.y) * PX_SIZE;
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
		if ((GLsizei)w <= 0 || (GLsizei)h <= 0) {
			glDisable(GL_SCISSOR_TEST);
			continue;
		}
		glScissor((GLint)round(__x), (GLint)round(__y), (GLsizei)w, (GLsizei)h);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform4fv(glGetUniformLocation(res::shaders::rectangle, "colour"),
			1, glm::value_ptr(quad.colour)); glUniform1f(glGetUniformLocation(res::shaders::rectangle, "noise"), 0.0f);
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
		object& tmp = res::objects.data()[i];
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
		for (int j = 0; tmp.hitbox && j < tmp.hitbox->size(); j++) {
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
	for (int i = 0; i < immune.size(); immune[i++]--);
	while (immune.size() > 0 && immune.front() <= 0) {
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
	case 1: angle += atan2(1.0, 3.0);	break;
	case 2: angle += atan2(1.0, 2.0);	break;
	case 3: angle += PI / 4.0;			break;
	case 4: angle += atan(2.0);			break;
	case 5: angle += atan(3.0);			break;
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
	if (victim->parent) {
		std::vector<node*>::iterator it = std::remove_if(
			victim->parent->children.begin(), victim->parent->children.end(),
			[victim](node* other) { return other == victim; });
		victim->parent->children.erase(it, victim->parent->children.end());
	}
	victim->parent = target;
	if (target)
		target->children.push_back(victim);
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

void node::destroy(void) {
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
