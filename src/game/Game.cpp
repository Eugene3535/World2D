#include <array>

#include <glad/glad.h>
#include <cglm/struct/cam.h>
#include "cglm/struct/affine-mat.h"

#include "files/Shader.hpp"
#include "files/TileMap.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/program/ShaderProgram.hpp"
#include "graphics/sprites/SpriteManager.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/level/Level.hpp"
#include "game/window/Viewport.hpp"
#include "game/Game.hpp"



Game::Game() noexcept
{

}


Game::~Game() = default;


bool Game::setup() noexcept
{
    m_viewport = std::make_unique<Viewport>();
	m_viewport->width = 800;
    m_viewport->height = 600;
    m_viewport->title = "Platformer 2D";
    m_viewport->view = &m_view;

	if (!m_window.open(m_viewport.get()))
		return false;

    if (!m_context.initialize())
        return false;

    m_view.initialize(m_viewport->width, m_viewport->height);

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

    while (m_window.isOpen())
	{
        glClear(GL_COLOR_BUFFER_BIT);

		m_window.pollEvents();

		float currentFrame = m_window.getElapsedTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        if (m_window.isKeyPressed('W'))
        {
            level.move({0, 5});
        }

        if (m_window.isKeyPressed('S'))
        {
            level.move({0, -5});
        }

        if (m_window.isKeyPressed('D'))
        {
            level.move({-5, 0});
        }

        if (m_window.isKeyPressed('A'))
        {
            level.move({5, 0});
        }

        auto projection = m_view.getProjectionMatrix();
        auto modelView = level.getMatrix();
        auto mvp = glms_mul(projection, modelView);
        m_view.updateUniformBuffer(mvp.raw);

        level.draw();

		m_window.display();
	}

	return 0;
}


