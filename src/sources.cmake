set(FILE_SRC_FILES
	src/files/TileMap.cpp
	src/files/TileMap.hpp
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
	# ${COMMON_SRC_FILES}
	${GRAPHICS_SRC_FILES}
	src/game/objects/GameObject.hpp
	src/game/main_loop/MainLoop.cpp
	src/game/main_loop/MainLoop.hpp
	src/game/Game.cpp
	src/game/Game.hpp
	src/main.cpp
)