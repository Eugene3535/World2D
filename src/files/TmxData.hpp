#pragma once

#include <string>
#include <variant>
#include <vector>

#include <cglm/struct/ivec2.h>


struct TmxObject
{
    struct Property
    {
        std::string                                 name;
        std::variant<bool, int, float, std::string> value;
    };

    std::string name;
    std::string type;
    ivec2s      position;
    ivec2s      size;

    std::vector<Property> properties;
};