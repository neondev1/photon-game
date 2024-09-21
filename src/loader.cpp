#include <filesystem>
#include <cstring>

#include "head.hpp"

namespace res::loader {
	std::vector<rect> textures[32];
	std::vector<box> hitboxes[32];
	std::vector<std::vector<object>> levels;
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

void res::loader::load_obj(void) {
	levels.clear();
	levels.push_back(std::vector<object>());
	levels.back().push_back(object(0.0, 0.0, 1, 1,
		object::enum_orientation::NONE, object::enum_type::NONE, TEX_PHOTON, NULL,
		(int)photon::enum_direction::E));
}

void res::loader::load_level(int level) {
	photon::deleting.clear();
	photon::nodes.clear();
	photon::photons.clear();
	object::invalidated.clear();
	res::objects.clear();
	res::objects.resize(res::loader::levels[level].size() - 1);
	memcpy(res::objects.data(),
		res::loader::levels.data()[level].data() + 1,
		std::min(res::objects.size(), res::loader::levels.data()[level].size() - 1) * sizeof(object));
	photon::photons.push_back(photon(
		TEX(0),
		res::loader::levels.data()[level].data()[0].x,
		res::loader::levels.data()[level].data()[0].y,
		(photon::enum_direction)res::loader::levels.data()[level].data()[0].data,
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
	object::selected = &res::objects.data()[1];
}

namespace gamestate {
	bool started = false;
	bool hardcore = false;
	int level = 0;
	int failures = 0;
	double time = 0.0;
	std::string save;
}
