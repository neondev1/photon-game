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

void res::loader::load_default(void) {
	levels.clear();
	levels.push_back(level());
	levels.back().objects.push_back(object(0.0, 0.0, 1, 1,
		object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL,
		(int)photon::enum_direction::E));
	levels.back().objects.push_back(object(0.0, 0.0, 640, 360,
		object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0));
}

bool res::loader::load_from_file(std::string path) {
	if (!std::filesystem::is_regular_file(path))
		return false;
	levels.clear();
	std::ifstream in(path);
	std::string str;
	while (std::getline(in, str)) {
		int x, y, dir;
		if (str.data()[0] == ' ') {
			levels.push_back(level());
			levels.back().hint = str.substr(1);
			in >> x >> y >> dir;
			if (in.fail() || dir < 0 || dir >= (int)photon::enum_direction::NONE) {
				in.close();
				return false;
			}
			levels.back().objects.push_back(object(x, y, 1, 1,
				object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL, dir));
			levels.back().objects.push_back(object(0.0, 0.0, 640, 360,
				object::enum_orientation::NONE, object::enum_type::NONE, NULL, NULL, 0));
			std::getline(in, str);
			continue;
		}
		int type;
		std::istringstream iss(str);
		iss >> type >> x >> y >> dir;
		if (iss.fail() || type < 0 || type > (int)object::enum_type::NONE
			|| dir < 0 || dir > (int)object::enum_orientation::NONE) {
			in.close();
			return false;
		}
		object::enum_type t = (object::enum_type)type;
		levels.back().objects.push_back(object(x, y, 20, 20,
			(object::enum_orientation)dir, t,
			res::loader::get_tex(t), res::loader::get_hbx(t), 0));
		if (t == object::enum_type::MOVING_WALL
			|| t == object::enum_type::MOVING_BLOCK
			|| t == object::enum_type::MOVING_CRYSTAL) {
			int x2, y2, data;
			iss >> x2 >> y2 >> data;
			if (iss.fail() || data < 0) {
				in.close();
				return false;
			}
			levels.back().objects.back().x2 = x2;
			levels.back().objects.back().y2 = y2;
			levels.back().objects.back().data = data;
		}
	}
	in.close();
	return true;
}

void res::loader::load_level(int level, bool randomize_orientation) {
	if (level >= res::loader::levels.size())
		return;
	photon::deleting.clear();
	photon::nodes.clear();
	photon::photons.clear();
	object::invalidated.clear();
	res::objects.clear();
	res::objects.resize(res::loader::levels[level].objects.size() - 1);
	std::memcpy(res::objects.data(),
		res::loader::levels.data()[level].objects.data() + 1,
		std::min(res::objects.size(), res::loader::levels.data()[level].objects.size() - 1) * sizeof(object));
	photon::photons.push_back(photon(
		TEX(0),
		res::loader::levels.data()[level].objects.data()[0].x,
		res::loader::levels.data()[level].objects.data()[0].y,
		(photon::enum_direction)res::loader::levels.data()[level].objects.data()[0].data,
		0, 0,
		NULL
	));
	object::groups.clear();
	for (int i = 0; i < res::objects.size(); i++) {
		object& obj = res::objects.data()[i];
		if (obj.data && (obj.type == object::enum_type::MOVING_WALL
			|| obj.type == object::enum_type::MOVING_BLOCK
			|| obj.type == object::enum_type::MOVING_CRYSTAL)) {
			while (obj.data > object::groups.size())
				object::groups.push_back(group());
			std::list<group>::iterator it = object::groups.begin();
			for (int j = 1; j < obj.data; j++)
				++it;
			it->add(&obj);
		}
	}
	object::selected = res::objects.size() > 1 ? &res::objects.data()[1] : NULL;
}

namespace gamestate {
	bool started = false;
	bool hardcore = false;
	int level = 0;
	int failures = 0;
	double time = 0.0;
	std::string save;
}
