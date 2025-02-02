#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

struct Attack {
	int range;
	int damage;
};

struct Creature {
	float health;
	float speed;
};

struct Dimension {
	int width;
	int height;
};

struct Experience {
	int exp;
};

struct Position {
	float x;
	float y;
};

struct Texture {

};

