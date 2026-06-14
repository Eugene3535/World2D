
set(FILE_SRC_FILES
	# src/files/Font.cpp
	src/files/Shader.cpp
	src/files/StbImage.cpp
	src/files/TileMap.cpp
)

set(FILE_HDR_FILES
	# src/files/Font.hpp
	src/files/Shader.hpp
	src/files/StbImage.hpp
	src/files/TileMap.hpp
)

set(GRAPHICS_SRC_FILES
	src/graphics/shaders/ShaderProgram.cpp
	src/graphics/texture/Texture2D.cpp
	src/graphics/vao/VertexBufferLayout.cpp
	src/graphics/transform/Transform2D.cpp
	src/graphics/camera/OrthoMatrix.cpp
	src/graphics/sprites/SpriteManager.cpp
)

set(GRAPHICS_HDR_FILES
	src/graphics/shaders/ShaderProgram.hpp
	src/graphics/texture/Texture2D.hpp
	src/graphics/vao/VertexBufferLayout.hpp
	src/graphics/sprites/SpriteManager.hpp
	src/graphics/transform/Transform2D.hpp
	src/graphics/camera/OrthoMatrix.hpp
)

set(GAME_SRC_FILES
	${FILE_SRC_FILES}
	${GRAPHICS_SRC_FILES}
	src/main.cpp
)

set(GAME_HDR_FILES
	src/common/allocator/MemoryAllocator.hpp
	src/common/allocator/MemoryAllocator.inl
	${FILE_HDR_FILES}
	${GRAPHICS_HDR_FILES}
)


set(SHADER_FILES
	src/shaders/tilemap.vert
	src/shaders/tilemap.frag
)

source_group("shaders" FILES ${SHADER_FILES})

set(GAME_ALL_FILES 
	${GAME_SRC_FILES} 
	${GAME_HDR_FILES}
)