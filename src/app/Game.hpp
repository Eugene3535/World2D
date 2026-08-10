#pragma once

#include "graphics/resources/GlResourceManager.hpp"
#include "app/window/WindowData.hpp"


struct Game final
{
    enum State
    {
        TitleScreen,
        Stage
    };

    WindowData windowData;
    GlResourceManager glResources;
};