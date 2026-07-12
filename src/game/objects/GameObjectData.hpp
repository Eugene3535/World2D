#pragma once

#include <map>
#include <string>
#include <utility>

#include <cglm/struct/vec2.h>



struct GameObjectData
{
    using AnimationRange = std::pair<uint32_t, uint32_t>;

    std::map<std::string, AnimationRange> animations;
    ivec2s position;
};
