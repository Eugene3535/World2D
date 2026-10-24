#pragma once

#include <cglm/struct/vec2.h>


struct WindowData final
{
    vec2s cursor;

    struct
    {
        int button;
        int action;
        int mods;
    } mouse;

    struct
    {
        int key;
        int scancode;
        int action;
        int mods;
    } keyboard;

    const class View* view = nullptr;

    ivec2s locationSize;
};