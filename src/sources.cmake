set(FILE_SRC_FILES
	src/files/provider/FileProvider.cpp
	src/files/provider/FileProvider.hpp
	src/files/tilemap/TileMap.cpp
	src/files/tilemap/TileMap.hpp
	src/files/assets/AssetManager.hpp
	src/files/assets/AssetManager.inl
)


set(COMMON_SRC_FILES
	src/common/Defines.hpp
	src/common/allocator/MemoryAllocator.hpp
	src/common/allocator/MemoryAllocator.inl
	src/common/action/Action.cpp
	src/common/action/Action.hpp
	src/common/action/ActionData.hpp
	src/common/action/ActionManager.cpp
	src/common/action/ActionManager.hpp
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
	${COMMON_SRC_FILES}
	${GRAPHICS_SRC_FILES}
	src/app/objects/GameObject.hpp
	src/app/context/Context.cpp
	src/app/context/Context.hpp
	src/app/loop/MainLoop.cpp
	src/app/loop/MainLoop.hpp
	src/app/game/Game.cpp
	src/app/game/Game.hpp
	src/main.cpp
)