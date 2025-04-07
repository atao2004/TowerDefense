#pragma once

#include <map>
#include "enemies.hpp"

const std::vector<std::pair<ENEMY_ID, int>>
DAY_1 = {
    {ENEMY_ID::ORC, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_2 = {
    {ENEMY_ID::ORC, 15}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_3 = {
    {ENEMY_ID::ORC, 15},
    {ENEMY_ID::ORC_ELITE, 10},
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_4 = {
    {ENEMY_ID::SKELETON_ARCHER, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_5 = {
    {ENEMY_ID::SKELETON, 30}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_6 = {
    {ENEMY_ID::SKELETON_ARCHER, 5},
    {ENEMY_ID::SKELETON, 10},
    {ENEMY_ID::SKELETON_ARCHER, 5},
    {ENEMY_ID::SKELETON, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_7 = {
    {ENEMY_ID::ORC, 5},
    {ENEMY_ID::ORC_ELITE, 5},
    {ENEMY_ID::SKELETON, 10},
    {ENEMY_ID::SKELETON_ARCHER, 5},
    {ENEMY_ID::ORC, 5},
    {ENEMY_ID::ORC_ELITE, 5},
    {ENEMY_ID::SKELETON, 10},
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_8 = {
    {ENEMY_ID::WEREBEAR, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_9 = {
    {ENEMY_ID::WEREBEAR, 15}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_10 = {
    {ENEMY_ID::WEREBEAR, 15},
    {ENEMY_ID::WEREWOLF, 10}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_11 = {
    {ENEMY_ID::SKELETON_ARCHER, 10},
    {ENEMY_ID::ORC_RIDER, 5}
};

const std::vector<std::pair<ENEMY_ID, int>>
DAY_12 = {
    {ENEMY_ID::SKELETON_ARCHER, 5},
    {ENEMY_ID::SLIME, 15},
    {ENEMY_ID::ORC_RIDER, 5},
    {ENEMY_ID::SLIME, 15}
};

const std::map<int, std::vector<std::pair<ENEMY_ID, int>>>
DAY_MAP = {
    {1, DAY_1},
    {2, DAY_8},
    {3, DAY_9},
    {4, DAY_10}
};
