#pragma once

#include <type_traits>
#include <unordered_map>

#include "files/FileProvider.hpp"
#include "graphics/resources/GlResourceManager.hpp"
#include "graphics/texture/Texture2D.hpp"


class AssetManager final
{
public:
    AssetManager(GlResourceManager& glResources) noexcept;
    ~AssetManager();

    template <class T>
    T* get(const std::string& name) noexcept
    {
        if constexpr (std::is_same_v<T, Texture2D>)
        {
            return findOrLoad<Texture2D>(name, m_textures);
        }

        return nullptr;
    }

    GlResourceManager* getResourceManager() noexcept;
    static AssetManager* getInstance() noexcept;

private:
    template <class T>
    T* findOrLoad(const std::string& name, std::unordered_map<std::string, T>& container) noexcept
    {
        if (auto it = container.find(name); it != container.end())
            return &it->second;

        if constexpr (std::is_same_v<T, Texture2D>)
        {
            auto textures           = m_glResources->getHandles(GlResourceManager::GLTexture2D, 1);
            auto [iterator, result] = container.try_emplace(name, textures[0]);

            if (!result)
            {
                m_glResources->destroyHandles(GlResourceManager::GLTexture2D, textures);

                return nullptr;
            }

            const auto filepath = FileProvider::findPathToFile(name);

            if (std::filesystem::exists(filepath))
            {
                if (iterator->second.loadFromFile(filepath))
                    return &iterator->second;
            }

            m_glResources->destroyHandles(GlResourceManager::GLTexture2D, textures);
            container.erase(iterator);
        }

        return nullptr;
    }


    GlResourceManager*                         m_glResources;
    std::unordered_map<std::string, Texture2D> m_textures;
};

#define Assets AssetManager::getInstance()