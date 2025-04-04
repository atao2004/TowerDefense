#pragma once

#include <map>
#include "common.hpp"

struct seed {
    TEXTURE_ASSET_ID texture;
    PLANT_ID plant;
    seed(TEXTURE_ASSET_ID texture, PLANT_ID plant) : texture(texture), plant(plant) {}
};

const std::map<int, seed>
SEED_MAP = {
    {0, seed(TEXTURE_ASSET_ID::SEED_0, PLANT_ID::PLANT_1)},
    {1, seed(TEXTURE_ASSET_ID::SEED_1, PLANT_ID::PLANT_2_PURPLE)},
    {2, seed(TEXTURE_ASSET_ID::SEED_2, PLANT_ID::PLANT_2_CYAN)},
    {3, seed(TEXTURE_ASSET_ID::SEED_3, PLANT_ID::PLANT_2)},
    {4, seed(TEXTURE_ASSET_ID::SEED_4, PLANT_ID::PLANT_3)},
    {5, seed(TEXTURE_ASSET_ID::SEED_5, PLANT_ID::PLANT_1)},
    {6, seed(TEXTURE_ASSET_ID::SEED_6, PLANT_ID::PLANT_1)},
    {7, seed(TEXTURE_ASSET_ID::SEED_7, PLANT_ID::PLANT_1)}
};
