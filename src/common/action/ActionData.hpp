#pragma once

#include <SFML/System/Vector2.hpp>

#include "common/Defines.hpp"

BEGIN_NAMESPACE_ACTION
    
struct Construction
{
    float duration; // in seconds
    float countdown;
    float* progress;
};


struct CollisionMask
{
    char* mask;
    uint32_t mapWidth;
};


struct Movement
{
    struct
    {
        sf::Vector2i src;
        sf::Vector2i dest;
        sf::Vector2f cur;
    } position;
    
    float timer;
    float velocity;
    
    CollisionMask* mask;
};

END_NAMESPACE