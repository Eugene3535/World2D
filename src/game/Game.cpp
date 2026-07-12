#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/struct/vec2.h>
#include <cglm/struct/cam.h>
#include "cglm/struct/affine-mat.h"

#include "files/Shader.hpp"
#include "files/TileMap.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/program/ShaderProgram.hpp"
#include "graphics/sprites/SpriteManager.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/level/Level.hpp"
#include "game/Game.hpp"



Game::Game() noexcept:
    m_level(nullptr)
{

}


Game::~Game() = default;


bool Game::initialize() noexcept
{
	if (!m_window.open("Platformer 2D", 800, 600, m_view))
		return false;

    m_view.initialize(800, 600);

	return true;
}


int Game::play() noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level(tilemap);

    if (!level.load())
        return 1;

    m_level = &level;

    ShaderProgram program;
    {
        std::array<Shader, 2> shaders;

        if (!shaders[0].loadFromFile("res/shaders/sprite.vert", GL_VERTEX_SHADER)) return 1;
        if (!shaders[1].loadFromFile("res/shaders/sprite.frag", GL_FRAGMENT_SHADER)) return 1;
        if (!program.link(shaders)) return 1;
    }
    glUseProgram(program.getHandle());

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
            vec2s position = { static_cast<float>(-startPosition.x), static_cast<float>(-startPosition.y) };
            level.setPosition(position);
            auto projection = m_view.getProjectionMatrix();
            auto modelView = level.getMatrix();
            auto mvp = glms_mul(projection, modelView);
            m_view.updateUniformBuffer(mvp.raw);
        }
        finishPosition = startPosition;

        level.draw();

		m_window.display();
	}

	return 0;
}


const MainWindow& Game::getWindow() const noexcept
{
    return m_window;
}


const View& Game::getView() const noexcept
{
    return m_view;
}


Level* Game::getLevel() noexcept
{
    return m_level;
}