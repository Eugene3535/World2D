set(FILE_SRC_FILES
	# src/files/Font.cpp
	# src/files/Font.hpp
	src/files/Shader.cpp
	src/files/Shader.hpp
	src/files/StbImage.cpp
	src/files/StbImage.hpp
	src/files/TileMap.cpp
	src/files/TileMap.hpp
)


set(COMMON_SRC_FILES
	src/common/allocator/MemoryAllocator.hpp
	src/common/allocator/MemoryAllocator.inl
)


set(GRAPHICS_SRC_FILES
	src/graphics/program/ShaderProgram.cpp
	src/graphics/program/ShaderProgram.hpp
	src/graphics/texture/Texture2D.cpp
	src/graphics/texture/Texture2D.hpp
	src/graphics/vao/VertexBufferLayout.cpp
	src/graphics/vao/VertexBufferLayout.hpp
	src/graphics/transform/Transform2D.cpp
	src/graphics/transform/Transform2D.hpp
	src/graphics/camera/View.cpp
	src/graphics/camera/View.hpp
	src/graphics/sprites/SpriteManager.cpp
	src/graphics/sprites/SpriteManager.hpp
	src/graphics/level/Level.cpp
	src/graphics/level/Level.hpp
)


set(SHADER_FILES
	src/shaders/sprite.vert
	src/shaders/sprite.frag
)
source_group("shaders" FILES ${SHADER_FILES})


set(GAME_SRC_FILES
	${FILE_SRC_FILES}
	${COMMON_SRC_FILES}
	${GRAPHICS_SRC_FILES}
	${SHADER_FILES}
	src/game/window/MainWindow.cpp
	src/game/window/MainWindow.hpp
	src/game/window/WindowData.hpp
	src/game/context/Context.cpp
	src/game/context/Context.hpp
	src/game/objects/GameObject.cpp
	src/game/objects/GameObject.hpp
	src/game/Game.cpp
	src/game/Game.hpp
	src/main.cpp
)