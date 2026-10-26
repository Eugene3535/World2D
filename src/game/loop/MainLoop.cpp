#define GLFW_INCLUDE_NONE

#include "cglm/struct/affine-mat.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "files/FileProvider.hpp"
#include "files/TmxLoader.hpp"
#include "game/Game.hpp"
#include "game/loop/MainLoop.hpp"
#include "game/ui/HeadUpDisplay.hpp"
#include "game/window/MainWindow.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/tilemap/TileMap.hpp"


MainLoop::MainLoop(MainWindow& window) noexcept : m_window(window)
{
}


MainLoop::~MainLoop() = default;


void MainLoop::operator()(Game& game) noexcept
{
    TmxLoader loader;

    if (!loader.loadFromFile("World-1.tmx"))
        return;

    TileMap tilemap(loader, game.glResources);

    if (!tilemap.create())
        return;

    const GLuint program = game.glResources.getProgram("sprite");
    glUseProgram(program);

    float lastFrame = 0.f;

    glClearColor(0.f, 0.f, 0.f, 1.f);

    const auto mapSize           = loader.getMapSize();
    const auto tileSize          = loader.getTileSize();
    game.windowData.locationSize = glms_ivec2_mul(mapSize, tileSize);

    while (m_window.isOpen())
    {
        glClear(GL_COLOR_BUFFER_BIT);

        m_window.pollEvents();

        const float currentFrame = m_window.getElapsedTime();
        const float deltaTime    = currentFrame - lastFrame;
        lastFrame                = currentFrame;

        HeadUpDisplay hud(game.windowData);
        hud.update(tilemap, deltaTime);

        const ivec2s windowSize = m_window.getSize();
        const vec2s  position   = tilemap.getPosition();
        const ivec4s viewport   = {-(int)position.x, -(int)position.y, windowSize.x, windowSize.y};

        tilemap.drawBackground(viewport);
        tilemap.drawForeground(viewport);
        m_window.display();
    }
}