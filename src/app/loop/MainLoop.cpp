#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "cglm/struct/affine-mat.h"

#include "files/TileMap.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/level/Level.hpp"
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
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return;

    Level level(tilemap);

    if (!level.load())
        return;

    GLuint program = game.programs["sprite"];
    glUseProgram(program);

	float lastFrame = 0.f;

    glClearColor(0.f, 0.f, 0.f, 1.f);

    ivec2s startPosition = { 32, 0 };
    ivec2s finishPosition = { 0, 0 };

    while (m_window.isOpen())
	{
        glClear(GL_COLOR_BUFFER_BIT);

		m_window.pollEvents();

		float currentFrame = m_window.getElapsedTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


        if (m_window.isKeyPressed(GLFW_KEY_W))
        {
            startPosition.y -= 5;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_S))
        {
            startPosition.y += 5;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_D))
        {
            startPosition.x += 5;
        }
        else if (m_window.isKeyPressed(GLFW_KEY_A))
        {
            startPosition.x -= 5;
        }

        if ( (finishPosition.x != startPosition.x) || (finishPosition.y != startPosition.y) )
        {
			auto view = game.windowData.view;

            vec2s position = { static_cast<float>(-startPosition.x), static_cast<float>(-startPosition.y) };
            level.setPosition(position);
            auto projection = view->getProjectionMatrix();
            auto modelView = level.getMatrix();
            auto mvp = glms_mul(projection, modelView);
            view->updateUniformBuffer(mvp.raw);
        }
        finishPosition = startPosition;

        level.draw();

		m_window.display();
	}
}