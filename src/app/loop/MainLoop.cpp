#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "cglm/struct/affine-mat.h"

#include "files/TmxLoader.hpp"
#include "files/FileProvider.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/tilemap/TileMap.hpp"
#include "app/window/MainWindow.hpp"
#include "app/Game.hpp"
#include "app/loop/MainLoop.hpp"



MainLoop::MainLoop(MainWindow& window) noexcept:
    m_window(window)
{

}


MainLoop::~MainLoop() = default;


void MainLoop::operator()(Game& game) noexcept
{
    TmxLoader loader;

    if (!loader.loadFromFile("Level-1.tmx"))
        return;

    TileMap tilemap(loader, game.glResources);

    if (!tilemap.create())
        return;

    const GLuint program = game.glResources.getProgram("sprite");
    glUseProgram(program);

	float lastFrame = 0.f;

    glClearColor(0.f, 0.f, 0.f, 1.f);

    ivec2s startPosition = { 32, 0 };
    ivec2s finishPosition = { 0, 0 };

    const auto* view = game.windowData.view;
    const auto& projection = view->getProjectionMatrix();

    while (m_window.isOpen())
	{
        glClear(GL_COLOR_BUFFER_BIT);

		m_window.pollEvents();

		const float currentFrame = m_window.getElapsedTime();
		const float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


        if (m_window.isKeyPressed(GLFW_KEY_W))
        {
            startPosition.y -= 600 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_S))
        {
            startPosition.y += 600 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_D))
        {
            startPosition.x += 600 * deltaTime;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_A))
        {
            startPosition.x -= 600 * deltaTime;
        }

        if ( (finishPosition.x != startPosition.x) || (finishPosition.y != startPosition.y) )
        {
            vec2s position = { static_cast<float>(-startPosition.x), static_cast<float>(-startPosition.y) };
            tilemap.setPosition(position);

            const auto modelView = tilemap.getMatrix();
            const auto mvp = glms_mul(projection, modelView);
            view->updateUniformBuffer(mvp.raw);
        }

        finishPosition = startPosition;

        const ivec2s windowSize = m_window.getSize();
        const ivec4s viewport = { startPosition.x, startPosition.y, windowSize.x, windowSize.y };

        tilemap.draw(viewport);

		m_window.display();
	}
}