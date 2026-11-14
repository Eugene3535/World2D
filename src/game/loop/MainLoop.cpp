#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cglm/struct/aabb2d.h>
#include <cglm/struct/affine-mat.h>
#include <glad/glad.h>
#include <imgui.h>

#include "files/BinaryLoader.hpp"
#include "files/TmxLoader.hpp"
#include "game/Game.hpp"
#include "game/assets/AssetManager.hpp"
#include "game/loop/MainLoop.hpp"
#include "game/ui/HeadUpDisplay.hpp"
#include "game/window/MainWindow.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/tilemap/TileMap.hpp"


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

    TmxLoader         loader;
    const std::string locationName("Location-2.tmx");

    if (!loader.loadFromFile(locationName))
        return;

    TileMap tilemap(loader);

    if (!tilemap.create())
        return;

    auto objects = loader.getObjects();

    const GLuint program = game.glResources.getProgram("sprite");
    glUseProgram(program);

    float lastFrame = 0.f;

    const auto mapSize           = loader.getMapSize();
    const auto tileSize          = loader.getTileSize();
    game.windowData.locationSize = glms_ivec2_mul(mapSize, tileSize);

    while (m_window->isOpen())
    {
        m_window->clear();
        m_window->pollEvents();

        const float currentFrame = m_window->getElapsedTime();
        const float deltaTime    = currentFrame - lastFrame;
        lastFrame                = currentFrame;

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
            }
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->display();
    }

    BinaryLoader binaryLoader(locationName);
    tilemap.saveToFile(binaryLoader);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}