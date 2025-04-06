#pragma once

#include <map>
#include "enemies.hpp"

const std::vector<std::pair<ENEMY_ID, int>>
DAY_1 = {
    {ENEMY_ID::WEREWOLF, 5},
    {ENEMY_ID::SKELETON, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_2 = {
    {ENEMY_ID::ORC_ELITE, 10},
    {ENEMY_ID::SKELETON_ARCHER, 15}
};

const std::map<int, std::vector<std::pair<ENEMY_ID, int>>>
DAY_MAP = {
    {1, DAY_1},
    {2, DAY_2}
};
