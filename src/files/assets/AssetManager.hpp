#pragma once

#include <unordered_map>
#include <type_traits>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Music.hpp>

#include "files/provider/FileProvider.hpp"


class AssetManager final
{
public:
    template<class T>
    T* get(const std::string& filename) noexcept;

    template<class T>
    void release(const std::string& filename) noexcept;

private:
    template<class T>
    T* tryLoad(const std::string& filename, std::unordered_map<std::string, T>& container) noexcept;

    template<class T>
    void tryRelease(const std::string& filename, std::unordered_map<std::string, T>& container) noexcept;

    std::unordered_map<std::string, sf::Texture>     m_textures;
    std::unordered_map<std::string, sf::Font>        m_fonts;
    std::unordered_map<std::string, sf::Shader>      m_shaders;
    std::unordered_map<std::string, sf::SoundBuffer> m_sounds;
    std::unordered_map<std::string, sf::Music>       m_musics;
};

#include "files/assets/AssetManager.inl"