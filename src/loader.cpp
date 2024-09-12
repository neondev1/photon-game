#include <filesystem>
#include <cstring>

#include "head.hpp"

namespace res::loader {
	std::vector<box> hitboxes[32];
	std::vector<rect> textures[32];
	std::vector<std::vector<object>> levels;
}

void res::loader::load_obj(void) {
}

void res::loader::load_level(int level) {
	photon::removing.clear();
	photon::nodes.clear();
	photon::photons.clear();
	res::objects.clear();
	res::objects.resize(res::loader::levels[level].size() - 1);
	memcpy(res::objects.data(),
		res::loader::levels[level].data() + 1,
		std::min(res::objects.size(), res::loader::levels[level].size() - 1) * sizeof object);
	photon::photons.push_back(photon(
		&res::loader::textures[0],
		res::objects.data()[0]._x,
		res::objects.data()[0]._y,
		(photon::enum_direction)res::objects.data()[0].offset,
		0,
		NULL
	));
}
