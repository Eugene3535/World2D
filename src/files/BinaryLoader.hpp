#pragma once

#include <memory>
#include <string>
#include <vector>

#include <cglm/struct/ivec2.h>


class BinaryLoader final
{
public:
    struct Data
    {
        struct TilePlane
        {
            struct Chunk
            {
                uint32_t offset; // offset in bytes in EBO (void*)
                uint32_t count;  // number of indices
                uint32_t width;  // in tiles
                uint32_t height; // in tiles
            };

            std::string           texture;
            std::vector<float>    vertices;
            std::vector<uint32_t> indices;
            std::vector<Chunk>    chunks;
        };

        TilePlane background;
        TilePlane foreground;
        ivec2s    mapSize;
        ivec2s    tileSize;
    };

    BinaryLoader(const std::string& filename) noexcept;
    ~BinaryLoader();

    bool                  save(const Data& data) const noexcept;
    std::unique_ptr<Data> load() const noexcept;

private:
    std::string m_filename;
};