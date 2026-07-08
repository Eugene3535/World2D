#include <array>

#include <glad/glad.h>
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



Game::Game() noexcept
{

}


Game::~Game() = default;


bool Game::initialize() noexcept
{
	m_windowData.width = 800;
    m_windowData.height = 600;
    m_windowData.title = "Platformer 2D";

	if (!m_window.open(m_windowData))
		return false;

    m_view.initialize(m_windowData.width, m_windowData.height);
    m_windowData.view = &m_view;

	return true;
}


int Game::play() noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level;

    if (!level.load(tilemap))
        return 1;

    ShaderProgram program;
    {
        std::array<Shader, 2> shaders;

        if (!shaders[0].loadFromFile("res/shaders/sprite.vert", GL_VERTEX_SHADER)) return 1;
        if (!shaders[1].loadFromFile("res/shaders/sprite.frag", GL_FRAGMENT_SHADER)) return 1;
        if (!program.link(shaders)) return 1;
    }
    glUseProgram(program.getHandle());

    float deltaTime = 0.f;
	float lastFrame = 0.f;

    glClearColor(0.f, 0.f, 0.f, 1.f);

    vec2s startPosition = { 32, 0 };
    vec2s finishPosition = startPosition;
    {
        level.setPosition(glms_vec2_negate(startPosition));
        auto projection = m_view.getProjectionMatrix();
        auto modelView = level.getMatrix();
        auto mvp = glms_mul(projection, modelView);
        m_view.updateUniformBuffer(mvp.raw);
    }

    while (m_window.isOpen())
	{
        glClear(GL_COLOR_BUFFER_BIT);

		m_window.pollEvents();

		float currentFrame = m_window.getElapsedTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        if (m_window.isKeyPressed('W'))
        {
            startPosition.y -= 5;
        }

        if (m_window.isKeyPressed('S'))
        {
            startPosition.y += 5;
        }

        if (m_window.isKeyPressed('D'))
        {
            startPosition.x += 5;
        }

        if (m_window.isKeyPressed('A'))
        {
            startPosition.x -= 5;
        }

        if ( (finishPosition.x != startPosition.x) || (finishPosition.y != startPosition.y) )
        {
            level.setPosition(glms_vec2_negate(startPosition));
            auto projection = m_view.getProjectionMatrix();
            auto modelView = level.getMatrix();
            auto mvp = glms_mul(projection, modelView);
            m_view.updateUniformBuffer(mvp.raw);
        }

        vec2s finishPosition = startPosition;

        level.draw();

		m_window.display();
	}

	return 0;
}


