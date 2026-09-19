#pragma once

#include <cstdint>
#include <memory>


// class Level:
//     public Transform2D
// {
// public:
//     Level(const class TileMap& tilemap, class GlResourceManager& resources) noexcept;
//     ~Level();

//     bool load() noexcept;
//     void draw() const noexcept;

//     const class TileMap& getTileMap() const noexcept;

// private:
//     bool loadTextures() noexcept;
//     void loadGraphics() noexcept;
//     void cleanup() noexcept;

//     const class TileMap&     m_tilemap;
//     class GlResourceManager& m_glResources;

//     struct TilePlane
//     {
// 		uint32_t texture;
// 		uint32_t VAO; // Vertex Array Object
// 		uint32_t VBO; // Vertex Buffer Object
//         uint32_t EBO; // Element Buffer Object
// 		uint32_t indexCount;
//     };

//     TilePlane m_background;
//     TilePlane m_foreground;
//     bool m_isLoaded;
// };