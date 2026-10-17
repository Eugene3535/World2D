#pragma once

#include "app/window/WindowData.hpp"
#include "graphics/resources/GlResourceManager.hpp"


struct Game final
{
    enum State
    {
        TitleScreen,
        Stage
    };

    WindowData        windowData;
    GlResourceManager glResources;
};