#pragma once

#include <unordered_map>
#include <string>

#include "app/window/WindowData.hpp"


struct Game final
{
    enum State
    {
        TitleScreen,
        Stage
    };

    WindowData windowData;

    std::unordered_map<std::string, uint32_t> programs;
};