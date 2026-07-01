#pragma once

#include <cstdint>
#include <string>

#include "graphics/camera/View.hpp"


struct Viewport
{
    std::string title;
    int32_t width;
    int32_t height;

    View* view;

    struct
    {
        char code;
        bool isPressed;
        bool isRepeated;
        bool isReleased;
    } key;

    struct
    {
        int button;
        bool isMoved;
        bool isPressed;
        bool isReleased;
    } mouse;
};