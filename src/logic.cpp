#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "head.hpp"
#include "logic.hpp"

bool interact(photon* const p, double dist, object* const obj,
	box const* const hitbox, int line, int tps, std::list<photon>::iterator* iter) {
	object tmp = *obj;
	switch (obj->type) {
	case object::enum_type::WALL:
	case object::enum_type::MOVING_WALL:
		p->direction = photon::enum_direction::NONE;
		break;
	case object::enum_type::DOOR:
		if (!obj->toggle)
			p->direction = photon::enum_direction::NONE;
		break;
	case object::enum_type::MIRROR_DOOR:
		if (obj->toggle)
			break;
		[[fallthrough]];
	case object::enum_type::MIRROR:
	case object::enum_type::DIAGONAL_MIRROR: {
		int reflected = 3 * ((int)obj->orientation % 8) - (int)p->direction;
		if (reflected < 0)
			reflected += 24;
		p->direction = (photon::enum_direction)reflected;
		break;
	}
	case object::enum_type::MIRROR_BLOCK:
		tmp.type = object::enum_type::MIRROR;
		tmp.orientation = line % 2 ? object::enum_orientation::N : object::enum_orientation::W;
		return interact(p, dist, &tmp, hitbox, line, tps, iter);
	case object::enum_type::GLASS_BLOCK:
		if (line % 2 == ((int)obj->orientation + 7) / 4 % 2) {
			if (p->medium && p->medium != obj
				&& (int)p->medium->type >= (int)object::enum_type::GLASS_BLOCK
				&& (int)p->medium->type <= (int)object::enum_type::PRISM) {
				int duration = tps < 32 ? 1 : 2;
				std::deque<object*>::iterator iter;
				if ((iter = std::find(p->interacted.begin(), p->interacted.end(), p->medium)) != p->interacted.end()) {
					if (p->immune[iter - p->interacted.begin()] < duration)
						p->immune[iter - p->interacted.begin()] = duration;
				}
				else {
					p->interacted.push_back(p->medium);
					p->immune.push_back(duration);
				}
			}
			int next = line + 1 == 4 ? 0 : line + 1;
			double norm_x = hitbox->y[line] - hitbox->y[next];
			double norm_y = hitbox->x[next] - hitbox->x[line];
			double intersect_x = p->_x + dist * p->velx(tps);
			double intersect_y = p->_y + dist * p->vely(tps);
			if (distance(intersect_x + norm_x, intersect_y + norm_y, p->_x, p->_y) <
				distance(intersect_x - norm_x, intersect_y - norm_y, p->_x, p->_y)) {
				norm_x = -norm_x;
				norm_y = -norm_y;
			}
			double angle = acos((p->velx(tps) * norm_x + p->vely(tps) * norm_y)
				/ (sqrt(p->velx(tps) * p->velx(tps) + p->vely(tps) * p->vely(tps))
					* sqrt(norm_x * norm_x + norm_y * norm_y)));
			double norm = atan2(norm_y, norm_x);
			double n1 = p->medium ? 1.5 : 1.0;
			double n2 = 2.5 - n1;
			if (n1 * sin(angle) / n2 > 1.0) {
				tmp.type = object::enum_type::MIRROR;
				return interact(p, dist, &tmp, hitbox, line, tps, iter);
			}
			double heading = norm + asin(n1 * sin(angle) / n2);
			double heading2 = norm - asin(n1 * sin(angle) / n2);
			if (heading < 0) heading += 2 * PI;
			if (heading2 < 0) heading2 += 2 * PI;
			if (distance(intersect_x + p->vel(tps) * cos(heading), intersect_y + p->vel(tps) * sin(heading), p->_x, p->_y) <
				distance(intersect_x + p->vel(tps) * cos(heading2), intersect_y + p->vel(tps) * sin(heading2), p->_x, p->_y))
				heading = heading2;
			photon tp = photon(NULL, 0.0, 0.0, photon::enum_direction::NONE, 0, NULL);
			photon::enum_direction best = (photon::enum_direction)0;
			double diff = 2 * PI;
			for (int i = 0; i < (int)photon::enum_direction::NONE; i++) {
				tp.direction = (photon::enum_direction)i;
				if (abs(tp.heading() - heading) < diff) {
					best = tp.direction;
					diff = abs(tp.heading() - heading);
				}
			}
			p->direction = best;
			p->medium = p->medium == obj ? NULL : obj;
		}
		else
			p->direction = photon::enum_direction::NONE;
		break;
	case object::enum_type::FIXED_BLOCK:
	case object::enum_type::MOVING_BLOCK: {
		// literally just copied from the section above lol
		if (p->medium && p->medium != obj
			&& (int)p->medium->type >= (int)object::enum_type::GLASS_BLOCK
			&& (int)p->medium->type <= (int)object::enum_type::PRISM) {
			int duration = tps < 32 ? 1 : 2;
			std::deque<object*>::iterator iter;
			if ((iter = std::find(p->interacted.begin(), p->interacted.end(), p->medium)) != p->interacted.end()) {
				if (p->immune[iter - p->interacted.begin()] < duration)
					p->immune[iter - p->interacted.begin()] = duration;
			}
			else {
				p->interacted.push_back(p->medium);
				p->immune.push_back(duration);
			}
			p->medium = obj;
			return false;
		}
		int next = line + 1 == 4 ? 0 : line + 1;
		double norm_x = hitbox->y[line] - hitbox->y[next];
		double norm_y = hitbox->x[next] - hitbox->x[line];
		double intersect_x = p->_x + dist * p->velx(tps);
		double intersect_y = p->_y + dist * p->vely(tps);
		if (distance(intersect_x + norm_x, intersect_y + norm_y, p->_x, p->_y) <
			distance(intersect_x - norm_x, intersect_y - norm_y, p->_x, p->_y)) {
			norm_x = -norm_x;
			norm_y = -norm_y;
		}
		double angle = acos((p->velx(tps) * norm_x + p->vely(tps) * norm_y)
			/ (sqrt(p->velx(tps) * p->velx(tps) + p->vely(tps) * p->vely(tps))
				* sqrt(norm_x * norm_x + norm_y * norm_y)));
		double norm = atan2(norm_y, norm_x);
		double n1 = p->medium ? 1.5 : 1.0;
		double n2 = 2.5 - n1;
		if (n1 * sin(angle) / n2 > 1.0) {
			tmp.type = object::enum_type::MIRROR_BLOCK;
			return interact(p, dist, &tmp, hitbox, line, tps, iter);
		}
		double heading = norm + asin(n1 * sin(angle) / n2);
		double heading2 = norm - asin(n1 * sin(angle) / n2);
		if (heading < 0) heading += 2 * PI;
		if (heading2 < 0) heading2 += 2 * PI;
		if (distance(intersect_x + p->vel(tps) * cos(heading), intersect_y + p->vel(tps) * sin(heading), p->_x, p->_y) <
			distance(intersect_x + p->vel(tps) * cos(heading2), intersect_y + p->vel(tps) * sin(heading2), p->_x, p->_y))
			heading = heading2;
		photon tp = photon(NULL, 0.0, 0.0, photon::enum_direction::NONE, 0, NULL);
		photon::enum_direction best = (photon::enum_direction)0;
		double diff = 2 * PI;
		for (int i = 0; i < (int)photon::enum_direction::NONE; i++) {
			tp.direction = (photon::enum_direction)i;
			if (abs(tp.heading() - heading) < diff) {
				best = tp.direction;
				diff = abs(tp.heading() - heading);
			}
		}
		p->direction = best;
		p->medium = p->medium == obj ? NULL : obj;
		break;
	}
	case object::enum_type::PRISM: {
		if ((int)p->direction % 6 == 0 && !p->medium) {
			p->medium = obj;
			break;
		}
		int next = line + 1 == 3 ? 0 : line + 1;
		double norm_x = hitbox->y[line] - hitbox->y[next];
		double norm_y = hitbox->x[next] - hitbox->x[line];
		double intersect_x = p->_x + dist * p->velx(tps);
		double intersect_y = p->_y + dist * p->vely(tps);
		if (distance(intersect_x + norm_x, intersect_y + norm_y, p->_x, p->_y) <
			distance(intersect_x - norm_x, intersect_y - norm_y, p->_x, p->_y)) {
			norm_x = -norm_x;
			norm_y = -norm_y;
		}
		double angle = acos((p->velx(tps) * norm_x + p->vely(tps) * norm_y)
			/ (sqrt(p->velx(tps) * p->velx(tps) + p->vely(tps) * p->vely(tps))
				* sqrt(norm_x * norm_x + norm_y * norm_y)));
		double norm = atan2(norm_y, norm_x);
		double n1 = p->medium ? 1.5 : 1.0;
		double n2 = 2.5 - n1;
		if (n1 * sin(angle) / n2 > 1.0) {
			object::enum_orientation orient;
			switch (line) {
			case 0: orient = object::enum_orientation::SE;	break;
			case 1: orient = object::enum_orientation::SW;	break;	
			case 2: orient = object::enum_orientation::N;	break;
			}
			tmp.orientation = orient;
			tmp.type = object::enum_type::MIRROR;
			return interact(p, dist, &tmp, hitbox, line, tps, iter);
		}
		double heading = norm + asin(n1 * sin(angle) / n2);
		double heading2 = norm - asin(n1 * sin(angle) / n2);
		if (heading < 0) heading += 2 * PI;
		if (heading2 < 0) heading2 += 2 * PI;
		if (distance(intersect_x + p->vel(tps) * cos(heading), intersect_y + p->vel(tps) * sin(heading), p->_x, p->_y) <
			distance(intersect_x + p->vel(tps) * cos(heading2), intersect_y + p->vel(tps) * sin(heading2), p->_x, p->_y))
			heading = heading2;
		photon tp = photon(NULL, 0.0, 0.0, photon::enum_direction::NONE, 0, NULL);
		photon::enum_direction best = (photon::enum_direction)0;
		double diff = 2 * PI;
		for (int i = 0; i < (int)photon::enum_direction::NONE; i++) {
			tp.direction = (photon::enum_direction)i;
			if (abs(tp.heading() - heading) < diff) {
				best = tp.direction;
				diff = abs(tp.heading() - heading);
			}
		}
		p->direction = best;
		p->medium = p->medium == obj ? NULL : obj;
		break;
	}
	case object::enum_type::SPLITTER:
	case object::enum_type::FIXED_SPLITTER: {
		node* n;
		if (!p->parent || p->parent->type != node::enum_node::SUPERPOS) {
			n = p->parent->add(node::enum_node::SUPERPOS);
			node::move(p, n);
		}
		else
			n = p->parent;
		photon::photons.push_back(photon(p->texture, p->_x, p->_y, p->direction, p->dc, n));
		photon* np = &photon::photons.back();
		np->parent->items.push_back(np);
		np->interacted.push_back(obj);
		np->immune.push_back(tps < 32 ? 1 : 2);
		tmp.type = object::enum_type::MIRROR;
		return interact(p, dist, &tmp, hitbox, line, tps, iter);
	}
	case object::enum_type::SPDC_CRYSTAL:
	case object::enum_type::MOVING_CRYSTAL: {
		if (!p->medium || p->medium->type != object::enum_type::SPDC_CRYSTAL)
			p->medium = obj;
		else {
			p->medium = NULL;
			p->dc++;
			int dir = (int)p->direction;
			photon::enum_direction dir1 = (photon::enum_direction)(dir == 0 ? 23 : dir - 1);
			photon::enum_direction dir2 = (photon::enum_direction)(dir == 23 ? 0 : dir + 1);
			node* n;
			if (!p->parent)
				n = NULL;
			else if (p->parent->type != node::enum_node::SPDC) {
				n = p->parent->add(node::enum_node::SPDC);
				node::move(p, n);
			}
			else
				n = p->parent;
			photon::photons.push_back(photon(p->texture, p->_x, p->_y, dir1, p->dc, n));
			photon* np = &photon::photons.back();
			np->parent->items.push_back(np);
			np->interacted.push_back(obj);
			np->immune.push_back(tps < 32 ? 1 : 2);
			p->direction = dir2;
		}
		break;
	}
	case object::enum_type::BOMB:
		if (!p->parent) {
			//fail
			return true;
		}
		else if (p->parent->type == node::enum_node::SUPERPOS) {
			p->destroy();
			*iter = photon::photons.erase(*iter);
			return true;
		}
		for (node* n = p->parent;; n = n->parent) {
			if (!n->parent) {
				// fail
				return true;
			}
			else if (n->parent->type == node::enum_node::SUPERPOS) {
				photon::removing.push_back(n);
				break;
			}
		}
		++(*iter);
		return true;
	case object::enum_type::SENSOR:
		if (!p->parent) {
			*iter = photon::photons.erase(*iter);
			return true;
		}
		for (node* n = p->parent; n; n = n->parent) {
			if (n->type == node::enum_node::SUPERPOS) {
				photon::removing.push_back(n);
				++(*iter);
				return true;
			}
		}
		p->destroy();
		*iter = photon::photons.erase(*iter);
		return true;
	default: // this shouldn't ever happen
		for (;;);
	}
	return false;
}

double intersect(double x1, double y1, double x2, double y2,
	double x3, double y3, double x4, double y4) {
	double x00 = x1, y00 = y1;
	double x01 = x2 - x1, y01 = y2 - y1;
	double x10 = x3, y10 = y3;
	double x11 = x4 - x3, y11 = y4 - y3;
	double d = x11 * y01 - x01 * y11;
	if (d == 0.0)
		return 2.0;
	double s = ((x00 - x10) * y01 - (y00 - y10) * x01) / d;
	if (s > 1.0 || s < 0.0)
		return 2.0;
	double t = -((y00 - y10) * x11 - (x00 - x10) * y11) / d;
	return (t >= 0.0 && t <= 1.0) ? t : 2.0;
}

void select(int key) {
	if (object::selected == NULL)
		return;
	object* best = NULL;
	object::enum_type unselectables[] = {
		object::enum_type::WALL,
		object::enum_type::DIAGONAL_MIRROR, object::enum_type::MIRROR_BLOCK,
		object::enum_type::FIXED_BLOCK, object::enum_type::PRISM,
		object::enum_type::FIXED_SPLITTER, object::enum_type::SPDC_CRYSTAL,
		object::enum_type::BOMB, object::enum_type::SENSOR,
		object::enum_type::NONE
	};
	object::enum_type* end = unselectables + sizeof(unselectables) / sizeof(object::enum_type);
	// I'm sorry
	switch (key) {
	case GLFW_KEY_W:
		for (int i = 0; i < res::objects.size(); i++) {
			object* obj = &res::objects.data()[i];
			if (std::find(unselectables, end, obj->type) != end)
				continue;
			if (obj->midy() <= object::selected->midy())
				continue;
			if (obj->midy() - object::selected->midy()
				< labs(obj->midx() - object::selected->midx()))
				continue;
			if (best == NULL)
				best = obj;
			else if (obj->midy() < best->midy())
				best = obj;
			else if (obj->midy() == best->midy()
				&& labs(obj->midx() - object::selected->midx())
				< labs(best->midx() - object::selected->midx()))
				best = obj;
		}
		if (best == NULL) {
			for (int i = 0; i < res::objects.size(); i++) {
				object* obj = &res::objects.data()[i];
				if (std::find(unselectables, end, obj->type) != end)
					continue;
				if (obj->midy() <= object::selected->midy())
					continue;
				if (best == NULL)
					best = obj;
				else if (labs(obj->midx() - object::selected->midx()) - (obj->midy() - object::selected->midy())
					< labs(best->midx() - object::selected->midx()) - (best->midy() - object::selected->midy()))
					best = obj;
				else if (labs(obj->midx() - object::selected->midx()) - (obj->midy() - object::selected->midy())
					== labs(best->midx() - object::selected->midx()) - (best->midy() - object::selected->midy())
					&& obj->midy() < best->midy())
					best = obj;
			}
		}
		break;
	case GLFW_KEY_A:
		for (int i = 0; i < res::objects.size(); i++) {
			object* obj = &res::objects.data()[i];
			if (std::find(unselectables, end, obj->type) != end)
				continue;
			if (obj->midx() >= object::selected->midx())
				continue;
			if (object::selected->midx() - obj->midx()
				< labs(obj->midy() - object::selected->midy()))
				continue;
			if (best == NULL)
				best = obj;
			else if (obj->midx() > best->midx())
				best = obj;
			else if (obj->midx() == best->midx()
				&& labs(obj->midy() - object::selected->midy())
				< labs(best->midy() - object::selected->midy()))
				best = obj;
		}
		if (best == NULL) {
			for (int i = 0; i < res::objects.size(); i++) {
				object* obj = &res::objects.data()[i];
				if (std::find(unselectables, end, obj->type) != end)
					continue;
				if (obj->midx() >= object::selected->midx())
					continue;
				if (best == NULL)
					best = obj;
				else if (labs(obj->midy() - object::selected->midy()) - (object::selected->midx() - obj->midx())
					< labs(best->midy() - object::selected->midy()) - (object::selected->midx() - best->midx()))
					best = obj;
				else if (labs(obj->midy() - object::selected->midy()) - (object::selected->midx() - obj->midx())
					== labs(best->midy() - object::selected->midy()) - (object::selected->midx() - best->midx())
					&& obj->midx() > best->midx())
					best = obj;
			}
		}
		break;
	case GLFW_KEY_S:
		for (int i = 0; i < res::objects.size(); i++) {
			object* obj = &res::objects.data()[i];
			if (std::find(unselectables, end, obj->type) != end)
				continue;
			if (obj->midy() >= object::selected->midy())
				continue;
			if (object::selected->midy() - obj->midy()
				< labs(obj->midx() - object::selected->midx()))
				continue;
			if (best == NULL)
				best = obj;
			else if (obj->midy() > best->midy())
				best = obj;
			else if (obj->midy() == best->midy()
				&& labs(obj->midx() - object::selected->midx())
				< labs(best->midx() - object::selected->midx()))
				best = obj;
		}
		if (best == NULL) {
			for (int i = 0; i < res::objects.size(); i++) {
				object* obj = &res::objects.data()[i];
				if (std::find(unselectables, end, obj->type) != end)
					continue;
				if (obj->midy() >= object::selected->midy())
					continue;
				if (best == NULL)
					best = obj;
				else if (labs(obj->midx() - object::selected->midx()) - (object::selected->midy() - obj->midy())
					< labs(best->midx() - object::selected->midx()) - (object::selected->midy() - best->midy()))
					best = obj;
				else if (labs(obj->midx() - object::selected->midx()) - (object::selected->midy() - obj->midy())
					== labs(best->midx() - object::selected->midx()) - (object::selected->midy() - best->midy())
					&& obj->midy() > best->midy())
					best = obj;
			}
		}
		break;
	case GLFW_KEY_D:
		for (int i = 0; i < res::objects.size(); i++) {
			object* obj = &res::objects.data()[i];
			if (std::find(unselectables, end, obj->type) != end)
				continue;
			if (obj->midx() <= object::selected->midx())
				continue;
			if (obj->midx() - object::selected->midx()
				< labs(obj->midy() - object::selected->midy()))
				continue;
			if (best == NULL)
				best = obj;
			else if (obj->midx() < best->midx())
				best = obj;
			else if (obj->midx() == best->midx()
				&& labs(obj->midy() - object::selected->midy())
				< labs(best->midy() - object::selected->midy()))
				best = obj;
		}
		if (best == NULL) {
			for (int i = 0; i < res::objects.size(); i++) {
				object* obj = &res::objects.data()[i];
				if (std::find(unselectables, end, obj->type) != end)
					continue;
				if (obj->midx() <= object::selected->midx())
					continue;
				if (best == NULL)
					best = obj;
				else if (labs(obj->midy() - object::selected->midy()) - (obj->midx() - object::selected->midx())
					< labs(best->midy() - object::selected->midy()) - (best->midx() - object::selected->midx()))
					best = obj;
				else if (labs(obj->midy() - object::selected->midy()) - (obj->midx() - object::selected->midx())
					== labs(best->midy() - object::selected->midy()) - (best->midx() - object::selected->midx())
					&& obj->midx() < best->midx())
					best = obj;
			}
		}
		break;
	}
	if (best != NULL)
		object::selected = best;
}
