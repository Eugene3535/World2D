set(FILE_SRC_FILES
	src/files/TileMap.cpp
	src/files/TileMap.hpp
)


set(COMMON_SRC_FILES
	src/common/allocator/MemoryAllocator.hpp
	src/common/allocator/MemoryAllocator.inl
)


set(GRAPHICS_SRC_FILES
	src/graphics/frames/FrameManager.cpp
	src/graphics/frames/FrameManager.hpp
	src/graphics/animation/Animation.cpp
	src/graphics/animation/Animation.hpp
	src/graphics/level/Level.cpp
	src/graphics/level/Level.hpp
)


set(GAME_SRC_FILES
	${FILE_SRC_FILES}
	# ${COMMON_SRC_FILES}
	${GRAPHICS_SRC_FILES}
	src/game/objects/GameObject.hpp
	src/game/Game.cpp
	src/game/Game.hpp
	src/main.cpp
)