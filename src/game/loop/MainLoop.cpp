#include <variant>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cglm/struct/aabb2d.h>
#include <cglm/struct/affine-mat.h>
#include <glad/glad.h>
#include <imgui.h>

#include "game/Game.hpp"
#include "game/assets/AssetManager.hpp"
#include "game/loop/MainLoop.hpp"
#include "game/ui/HeadUpDisplay.hpp"
#include "game/window/MainWindow.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/tilemap/TileMap.hpp"

// just for sprite example
#include "graphics/animation/sprites/Sprite.hpp"
#include "graphics/animation/sprites/SpriteDataStorage.hpp"
#include <array>
#include <graphics/vao/VertexBufferLayout.hpp>


MainLoop::MainLoop(MainWindow& window) noexcept :
    m_window(&window)
{
}


MainLoop::~MainLoop() = default;


void MainLoop::operator()(Game& game) noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window->getGLFWHandle(), true);
    ImGui_ImplOpenGL3_Init(nullptr);

    AssetManager assets(game.glResources);


    const std::string locationName("Location-2");
    TileMap           tilemap(locationName);

    if (!tilemap.open())
        return;

    auto objects = tilemap.getObjects();

    const GLuint program = game.glResources.getProgram("sprite");
    glUseProgram(program);

    float lastFrame = 0.f;

    const auto mapSize           = tilemap.getMapSize();
    const auto tileSize          = tilemap.getTileSize();
    game.windowData.locationSize = glms_ivec2_mul(mapSize, tileSize);

    // Sprite example demo
    const auto spriteTexture       = Assets->get<Texture2D>("Vortex.png");
    auto       spriteVertexBuffers = game.glResources.getHandles(GlResourceManager::GLBuffer, 1);
    auto       spriteVertexArrays  = game.glResources.getHandles(GlResourceManager::GLVertexArray, 1);

    if (!spriteTexture || spriteVertexBuffers.empty() || spriteVertexArrays.empty())
        return;

    SpriteDataStorage spriteStorage;
    spriteStorage.createSprite("Vortex", spriteTexture->getHandle());
    spriteStorage.pushVerticesOnGPU(spriteVertexBuffers[0]);

    const std::array<VertexBufferLayout::Attribute, 1> attributes = {VertexBufferLayout::Attribute::Float4};
    VertexBufferLayout                                 layout(attributes);
    layout.createVertexInputState(spriteVertexArrays[0], spriteVertexBuffers[0]);

    auto spriteData = spriteStorage.getSprite("Vortex");

    if (!spriteData)
        return;

    Sprite      sprite(spriteData.value());
    const auto& currentWorldMatrix = game.windowData.view->getProjectionMatrix();
    sprite.setOrigin(spriteTexture->getSize().x * 0.5f, spriteTexture->getSize().y * 0.5f);
    sprite.setScale(0.5f, 0.5f);
    const vec2s spritePosition{655.f, 270.f};
    sprite.setPosition(spritePosition);

    while (m_window->isOpen())
    {
        m_window->clear();
        m_window->pollEvents();

        const float currentFrame = m_window->getElapsedTime();
        const float deltaTime    = currentFrame - lastFrame;
        lastFrame                = currentFrame;

        // sprite example demo
        sprite.setPosition(glms_vec2_add(spritePosition, tilemap.getPosition()));
        sprite.rotate(deltaTime * 10);
        const auto& model = sprite.getMatrix();
        auto        mvp   = glms_mul(currentWorldMatrix, model);
        game.windowData.view->updateUniformBuffer(mvp.raw);
        sprite.draw(spriteVertexArrays[0]);

        HeadUpDisplay hud(game.windowData);
        hud.update(tilemap, deltaTime);

        const ivec2s windowSize  = m_window->getSize();
        const vec2s  position    = glms_vec2_negate(tilemap.getPosition());
        const vec2s  worldCoords = glms_vec2_add(position, game.windowData.cursor);
        const ivec4s viewport    = {static_cast<int>(position.x),
                                    static_cast<int>(position.y),
                                    static_cast<int>(position.x) + windowSize.x,
                                    static_cast<int>(position.y) + windowSize.y};

        tilemap.drawBackground(viewport);
        tilemap.drawForeground(viewport);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Debug Info");
        ImGui::Text("Window size: (%d, %d)", windowSize.x, windowSize.y);
        ImGui::Text("Viewport: (%d, %d, %d, %d)", viewport.x, viewport.y, viewport.z, viewport.w);
        ImGui::Text("Local mouse position: (%.1f, %.1f)", game.windowData.cursor.x, game.windowData.cursor.y);
        ImGui::Text("Tilemap position: (%.1f, %.1f)", -position.x, -position.y);
        ImGui::Text("World cursor position: (%.1f, %.1f)", worldCoords.x, worldCoords.y);

        for (const auto& object : objects)
        {
            vec2s aabb[2];
            aabb[0].x = static_cast<float>(object.position.x);
            aabb[0].y = static_cast<float>(object.position.y);
            aabb[1].x = static_cast<float>(object.position.x + object.size.x);
            aabb[1].y = static_cast<float>(object.position.y + object.size.y);

            if (glms_aabb2d_point(aabb, worldCoords))
            {
                ImGui::Text("Found intersection with object: %s", object.name.c_str());

                if (!object.properties.empty())
                {
                    auto& property = object.properties[0];

                    if (const auto* s = std::get_if<std::string>(&property.value))
                    {
                        ImGui::Text("Object info: %s", s->c_str());
                    }
                }
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->display();
    }

    tilemap.save();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}