#ifndef _LOGIC_HPP_
#define _LOGIC_HPP_

#include "head.hpp"

inline double distance(double x1, double y1, double x2, double y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool interact(photon* const p, double dist, object* const obj,
	box const* const hitbox, int line, int tps, std::list<photon>::iterator* iter);
double intersect(double x1, double y1, double x2, double y2,
	double x3, double y3, double x4, double y4);
void select(int key);

#endif // _LOGIC_HPP_
