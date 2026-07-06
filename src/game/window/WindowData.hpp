#pragma once

#include <cstdint>
#include <string>

#include "graphics/camera/View.hpp"


struct WindowData
{
    std::string title;
    int32_t width;
    int32_t height;

    View view;

    struct
    {
        char code;
        bool isPressed;
        bool isRepeated;
        bool isReleased;
    } key;
};