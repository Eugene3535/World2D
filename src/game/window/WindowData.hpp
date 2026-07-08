#pragma once

#include <cstdint>
#include <string>


struct WindowData
{
    std::string title;
    int32_t width;
    int32_t height;

    void* view;
    void* ui;

    struct
    {
        char code;
        bool isPressed;
        bool isRepeated;
        bool isReleased;
    } key;
};