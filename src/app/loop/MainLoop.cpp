#define GLFW_INCLUDE_NONE

#include "cglm/struct/affine-mat.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "app/Game.hpp"
#include "app/loop/MainLoop.hpp"
#include "app/window/MainWindow.hpp"
#include "files/FileProvider.hpp"
#include "files/TmxLoader.hpp"
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

    ivec2s startPosition  = {32, 0};
    ivec2s finishPosition = {0, 0};

    const auto* view       = game.windowData.view;
    const auto& projection = view->getProjectionMatrix();

    const auto mapSize         = loader.getMapSize();
    const auto tileSize        = loader.getTileSize();
    const auto mapSizeInPixels = glms_ivec2_mul(mapSize, tileSize);

    while (m_window.isOpen())
    {
        glClear(GL_COLOR_BUFFER_BIT);

        m_window.pollEvents();

        const float currentFrame = m_window.getElapsedTime();
        const float deltaTime    = currentFrame - lastFrame;
        lastFrame                = currentFrame;

        if (m_window.isKeyPressed(GLFW_KEY_W))
        {
            startPosition.y -= 6000 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_S))
        {
            startPosition.y += 6000 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_D))
        {
            startPosition.x += 6000 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_A))
        {
            startPosition.x -= 6000 * deltaTime;
        }

        const ivec2s windowSize = m_window.getSize();
        if (startPosition.x < 1) startPosition.x = 0;
        if (startPosition.y < 1) startPosition.y = 0;
        if ((startPosition.x + windowSize.x) > mapSizeInPixels.x) startPosition.x = mapSizeInPixels.x - windowSize.x;
        if ((startPosition.y + windowSize.y) > mapSizeInPixels.y) startPosition.y = mapSizeInPixels.y - windowSize.y;

        const ivec4s viewport   = {startPosition.x, startPosition.y, windowSize.x, windowSize.y};

        if ((finishPosition.x != startPosition.x) || (finishPosition.y != startPosition.y))
        {
            vec2s position = {static_cast<float>(-startPosition.x), static_cast<float>(-startPosition.y)};
            tilemap.setPosition(position);

            const auto modelView = tilemap.getMatrix();
            const auto mvp       = glms_mul(projection, modelView);
            view->updateUniformBuffer(mvp.raw);
        }

        finishPosition = startPosition;

        tilemap.draw(viewport);

        m_window.display();
    }
}