#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "head.hpp"

namespace res::loader {
	std::vector<rect> textures[32];
	std::vector<box> hitboxes[32];
	std::vector<level> levels;
}

// Some #defines to make life easier
#define TEX(x) &res::loader::textures[x]
#define HBX(x) &res::loader::hitboxes[x]

#define TEX_PHOTON  	TEX(0)

#define TEX_WALL    	TEX(1)
#define TEX_DOOR    	TEX(2)
#define TEX_MOV_WALL	TEX(4)
#define TEX_ROT_MIRROR	TEX(5)
#define TEX_DMIRROR 	TEX(13)
#define TEX_MIRRORBLOCK	TEX(15)
#define TEX_MIRRORDOOR	TEX(16)
#define TEX_ROT_BLOCK	TEX(18)
#define TEX_FBLOCK  	TEX(22)
#define TEX_MOV_BLOCK	TEX(23)
#define TEX_PRISM   	TEX(24)
#define TEX_SPDC    	TEX(25)
#define TEX_MOV_SPDC	TEX(26)
#define TEX_SPLITTER	TEX(27)
#define TEX_BOMB    	TEX(29)
#define TEX_SENSOR  	TEX(31)

#define HBX_WALL    	NULL
#define HBX_DOOR    	NULL
#define HBX_MOV_WALL	NULL
#define HBX_ROT_MIRROR	HBX(0)
#define HBX_DMIRROR 	HBX(8)
#define HBX_MIRRORBLOCK	NULL
#define HBX_MIRRORDOOR	NULL
#define HBX_ROT_BLOCK	HBX(10)
#define HBX_FBLOCK  	NULL
#define HBX_MOV_BLOCK	NULL
#define HBX_PRISM   	HBX(14)
#define HBX_SPDC    	NULL
#define HBX_MOV_SPDC	NULL
#define HBX_SPLITTER	HBX(15)
#define HBX_BOMB    	NULL
#define HBX_SENSOR  	NULL

std::vector<rect>* res::loader::get_tex(object::enum_type type) {
	switch (type) {
	case object::enum_type::WALL:
		return TEX_WALL;
	case object::enum_type::DOOR:
		return TEX_DOOR;
	case object::enum_type::MOVING_WALL:
		return TEX_MOV_WALL;
	case object::enum_type::MIRROR:
		return TEX_ROT_MIRROR;
	case object::enum_type::DIAGONAL_MIRROR:
		return TEX_DMIRROR;
	case object::enum_type::MIRROR_BLOCK:
		return TEX_MIRRORBLOCK;
	case object::enum_type::MIRROR_DOOR:
		return TEX_MIRRORDOOR;
	case object::enum_type::GLASS_BLOCK:
		return TEX_ROT_MIRROR;
	case object::enum_type::FIXED_BLOCK:
		return TEX_FBLOCK;
	case object::enum_type::MOVING_BLOCK:
		return TEX_MOV_BLOCK;
	case object::enum_type::PRISM:
		return TEX_PRISM;
	case object::enum_type::SPDC_CRYSTAL:
		return TEX_SPDC;
	case object::enum_type::MOVING_CRYSTAL:
		return TEX_MOV_SPDC;
	case object::enum_type::SPLITTER:
		return TEX_SPLITTER;
	case object::enum_type::BOMB:
		return TEX_BOMB;
	case object::enum_type::SENSOR:
		return TEX_SENSOR;
	default:
		return NULL;
	}
}

std::vector<box>* res::loader::get_hbx(object::enum_type type) {
	switch (type) {
	case object::enum_type::MIRROR:
		return HBX_ROT_MIRROR;
	case object::enum_type::DIAGONAL_MIRROR:
		return HBX_DMIRROR;
	case object::enum_type::GLASS_BLOCK:
		return HBX_ROT_BLOCK;
	case object::enum_type::PRISM:
		return HBX_PRISM;
	case object::enum_type::SPLITTER:
		return HBX_SPLITTER;
	default:
		return NULL;
	}
}

const rect& res::loader::background(int level) {
	const object& obj = levels.data()[level].objects.data()[0];
	static rect r = { 0, 0, 640, 320, vec(0.08f, 0.08f, 0.08f, 1.0f), 5.0f };
	std::memcpy(&r.colour.r, &obj.x1, sizeof(float));
	std::memcpy(&r.colour.g, &obj.x2, sizeof(float));
	std::memcpy(&r.colour.b, &obj.y1, sizeof(float));
	r.colour.a = 1.0f;
	std::memcpy(&r.noise, &obj.y2, sizeof(float));
	return r;
}

void res::loader::set_background(int level, const vec& colour, float noise) {
	object& obj = levels.data()[level].objects.data()[0];
	std::memcpy(&obj.x1, &colour.r, sizeof(int));
	std::memcpy(&obj.x2, &colour.g, sizeof(int));
	std::memcpy(&obj.y1, &colour.b, sizeof(int));
	std::memcpy(&obj.y2, &noise, sizeof(int));
}

void res::loader::load_default(void) {
	levels.clear();
	levels.push_back(level());
	levels.back().objects.push_back(object(0.0, 0.0, 1, 1,
		object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL,
		(int)photon::enum_direction::E, false));
	levels.back().objects.push_back(object(0.0, 0.0, 640, 360,
		object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0, false));
	for (int i = 0; i < levels.size(); i++)
		set_background(i, object::default_colour, object::default_noise);
}

bool res::loader::load_from_file(std::string path) {
	if (!std::filesystem::is_regular_file(path))
		return false;
	std::vector<level> temp;
	std::ifstream in(path);
	std::string name;
	std::getline(in, name);
	std::string str;
	try {
		while (std::getline(in, str)) {
			int x, y, dir;
			if (str.data()[0] == ' ') {
				temp.push_back(level());
				int hint_length = std::stoi(str.substr(1));
				for (int i = 0; i < hint_length; i++) {
					std::string s;
					std::getline(in, s);
					temp.back().hint += s;
					if (i < hint_length - 1)
						temp.back().hint += '\n';
				}
				int r, g, b, noise;
				in >> x >> y >> dir >> r >> g >> b >> noise;
				if (in.fail() || dir < 0 || dir >= (int)photon::enum_direction::NONE) {
					in.close();
					return false;
				}
				temp.back().objects.push_back(object(x, y, 1, 1,
					object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL, dir, false));
				temp.back().objects.back().x1 = r;
				temp.back().objects.back().x2 = g;
				temp.back().objects.back().y1 = b;
				temp.back().objects.back().y2 = noise;
				temp.back().objects.push_back(object(0.0, 0.0, 640, 360,
					object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0, false));
				std::getline(in, str);
				continue;
			}
			int type, randomize;
			std::istringstream iss(str);
			iss >> type >> x >> y >> dir >> randomize;
			if (iss.fail() || type < 0 || type >(int)object::enum_type::NONE
				|| dir < 0 || dir >(int)object::enum_orientation::NONE) {
				in.close();
				return false;
			}
			object::enum_type t = (object::enum_type)type;
			temp.back().objects.push_back(object(x, y, 20, 20,
				(object::enum_orientation)dir, t,
				res::loader::get_tex(t), res::loader::get_hbx(t),
				0, randomize != 0));
			if (t == object::enum_type::MOVING_WALL
				|| t == object::enum_type::MOVING_BLOCK
				|| t == object::enum_type::MOVING_CRYSTAL) {
				int x2, y2, data;
				iss >> x2 >> y2 >> data;
				if (iss.fail() || data < 0) {
					in.close();
					return false;
				}
				temp.back().objects.back().x2 = x2;
				temp.back().objects.back().y2 = y2;
				temp.back().objects.back().data = data;
			}
			if (t == object::enum_type::DOOR
				|| t == object::enum_type::MIRROR_DOOR) {
				int data;
				iss >> data;
				if (iss.fail() || data < 0) {
					in.close();
					return false;
				}
				temp.back().objects.back().data = data;
			}
		}
	}
	catch (...) {
		in.close();
		return false;
	}
	in.close();
	levels.clear();
	levels.resize(temp.size());
	// Can't use `memcpy`; need to call copy constructors for `vector`s
	std::copy(temp.begin(), temp.end(), levels.begin());
	game::name = name;
	game::custom = true;
	return true;
}

void res::loader::load_level(int level, bool randomize_orientation) {
	if (level >= res::loader::levels.size())
		return;
	photon::deleting.clear();
	photon::nodes.clear();
	photon::photons.clear();
	object::invalidated.clear();
	object::temp_tex.clear();
	object::selected = NULL;
	res::objects.clear();
	res::objects.resize(res::loader::levels[level].objects.size() - 1);
	std::memcpy(res::objects.data(),
		res::loader::levels.data()[level].objects.data() + 1,
		std::min(res::objects.size(), res::loader::levels.data()[level].objects.size() - 1) * sizeof(object));
	const int p_x = res::loader::levels.data()[level].objects.data()[0].x;
	const int p_y = res::loader::levels.data()[level].objects.data()[0].y;
	photon::photons.push_back(photon(
		TEX_PHOTON, p_x, p_y,
		(photon::enum_direction)res::loader::levels.data()[level].objects.data()[0].data,
		0, 0, NULL
	));
	game::sensors = 0;
	object::groups.clear();
	static constexpr object::enum_type toggleable[] = {
		object::enum_type::DOOR, object::enum_type::MIRROR_DOOR,
		object::enum_type::MOVING_WALL, object::enum_type::MOVING_BLOCK, object::enum_type::MOVING_CRYSTAL
	};
	static const object::enum_type* t_end = toggleable + sizeof(toggleable) / sizeof(object::enum_type);
	for (int i = 0; i < res::objects.size(); i++) {
		object& obj = res::objects.data()[i];
		if (obj.data && std::find(toggleable, t_end, obj.type) != t_end) {
			while (obj.data > object::groups.size())
				object::groups.push_back(group());
			std::list<group>::iterator it = object::groups.begin();
			for (int j = 1; j < obj.data; j++)
				++it;
			it->add(&obj);
		}
		else if (obj.type == object::enum_type::SENSOR)
			game::sensors++;
		if ((randomize_orientation && obj.randomize)
			|| obj.orientation == object::enum_orientation::NONE) {
			if (obj.type == object::enum_type::MIRROR)
				obj.orientation = (object::enum_orientation)(mix32_rand(16));
			else if (obj.type == object::enum_type::GLASS_BLOCK)
				obj.orientation = (object::enum_orientation)(mix32_rand(8) * 2);
		}
	}
	game::activated = 0;
	static constexpr object::enum_type unselectables[] = {
		object::enum_type::WALL,
		object::enum_type::DIAGONAL_MIRROR, object::enum_type::MIRROR_BLOCK,
		object::enum_type::FIXED_BLOCK, object::enum_type::PRISM,
		object::enum_type::SPDC_CRYSTAL,
		object::enum_type::BOMB, object::enum_type::SENSOR,
		object::enum_type::NONE
	};
	static const object::enum_type* u_end = unselectables + sizeof(unselectables) / sizeof(object::enum_type);
	double best = DBL_MAX;
	for (int i = 1; i < res::objects.size(); i++) {
		double dist = 0.0;
		if (std::find(unselectables, u_end, res::objects.data()[i].type) == u_end
			&& (dist = obj_dist(p_x, p_y, res::objects.data()[i])) < best) {
			best = dist;
			object::selected = &res::objects.data()[i];
		}
	}
	if (!res::loader::levels.data()[level].hint_seen
		&& !res::loader::levels.data()[level].hint.empty()) {
		game::hint = true;
		res::loader::levels.data()[level].hint_seen = true;
	}
}

namespace game {
	bool started = false;
	bool hint = false;
	bool hardcore = false;
	int level = 0;
	int failures = 0;
	object* reason;
	int sensors = 0;
	int activated = 0;
	double time = 0.0;
	std::string save;
	std::string name;
	bool custom = false;
}
