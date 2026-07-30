

template<class T>
T* AssetManager::get(const std::string& filename) noexcept
{
//  Textures
	if constexpr (std::is_same_v<T, sf::Texture>)
		return tryLoad<sf::Texture>(filename, m_textures);

//  Fonts
	if constexpr (std::is_same_v<T, sf::Font>)
		return tryLoad<sf::Font>(filename, m_fonts);
	
//  Sounds
	if constexpr (std::is_same_v<T, sf::SoundBuffer>)
		return tryLoad<sf::SoundBuffer>(filename, m_sounds);

//  Music
	if constexpr (std::is_same_v<T, sf::Music>)
		return tryLoad<sf::Music>(filename, m_musics);

//  Shaders    
	if constexpr (std::is_same_v<T, sf::Shader>)
		return tryLoad<sf::Shader>(filename, m_shaders);

	return nullptr;
}


template<class T>
void AssetManager::release(const std::string& filename) noexcept
{
//  Textures
	if constexpr (std::is_same_v<T, sf::Texture>)
		tryRelease<sf::Texture>(filename, m_textures);
	
//  Fonts
	if constexpr (std::is_same_v<T, sf::Font>)
		tryRelease<sf::Font>(filename, m_fonts);

//  Shaders
	if constexpr (std::is_same_v<T, sf::Shader>)
		tryRelease<sf::Shader>(filename, m_shaders);

//  Sounds        
	if constexpr (std::is_same_v<T, sf::SoundBuffer>)
		tryRelease<sf::SoundBuffer>(filename, m_sounds);

//  Music
	if constexpr (std::is_same_v<T, sf::Music>)
		tryRelease<sf::Music>(filename, m_musics);
}


template<class T>
T* AssetManager::tryLoad(const std::string& filename, std::unordered_map<std::string, T>& container) noexcept
{
	if (auto it = container.find(filename); it != container.end())
		return &it->second;

	auto [iterator, result] = container.try_emplace(filename);

	if (result)
	{
//  sf::Music case               
		if constexpr (std::is_same_v<T, sf::Music>)
		{
			if (const std::string filepath = FileProvider::getPathToFile(filename); !iterator->second.openFromFile(filepath))
			{
				container.erase(iterator);

				return nullptr;
			}
		}

//  sf::Image, sf::Texture, sf::Font etc...                     
		if (const std::string filepath = FileProvider::getPathToFile(filename); !iterator->second.loadFromFile(filepath))
		{
			container.erase(iterator);

			return nullptr;
		}

		return &iterator->second;
	}

	return nullptr;
}


template<class T>
void AssetManager::tryRelease(const std::string& filename, std::unordered_map<std::string, T>& container) noexcept
{
	if (auto it = container.find(filename); it != container.end())
		container.erase(it);
}