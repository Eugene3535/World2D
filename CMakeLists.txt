cmake_minimum_required(VERSION 3.31.1)

project(Platformer LANGUAGES C CXX)

set(BUILD_SHARED_LIBS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(EXTERNAL_SOURCE_DIR $ENV{External})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(CMAKE_BUILD_TYPE)
    string(APPEND CMAKE_RUNTIME_OUTPUT_DIRECTORY "/${CMAKE_BUILD_TYPE}")
	string(APPEND CMAKE_LIBRARY_OUTPUT_DIRECTORY "/${CMAKE_BUILD_TYPE}")
	string(APPEND CMAKE_ARCHIVE_OUTPUT_DIRECTORY "/${CMAKE_BUILD_TYPE}")
endif()

if(MSVC)
	string(APPEND CMAKE_RUNTIME_OUTPUT_DIRECTORY "/bin")
	string(APPEND CMAKE_LIBRARY_OUTPUT_DIRECTORY "/lib")
	string(APPEND CMAKE_ARCHIVE_OUTPUT_DIRECTORY "/lib")
else()
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

set(SRC_FILES
	src/main.cpp
)

set(HDR_FILES
	src/common/MemoryAllocator.hpp
	src/common/MemoryAllocator.inl
	src/Animation.hpp
	src/Bullet.hpp
	src/Enemy.hpp
	src/Entity.hpp
	src/HealthBar.hpp
	src/Level.hpp
	src/MovingPlatform.hpp
	src/Player.hpp
)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES})
add_subdirectory(${EXTERNAL_SOURCE_DIR}/sfml sfml)
add_subdirectory(${EXTERNAL_SOURCE_DIR}/magic_enum magic_enum)

target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/src
	${EXTERNAL_SOURCE_DIR}/rapidxml
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	SFML::System
	SFML::Window
	SFML::Graphics
	SFML::Audio
	magic_enum::magic_enum
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /GR-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti)# -fno-exceptions (rapidxml throw errors fix)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if(MSVC)
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${DUNEII_ALL_FILES})
	set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

	set_target_properties(${PROJECT_NAME} PROPERTIES
		VS_DEBUGGER_WORKING_DIRECTORY_DEBUG          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
		VS_DEBUGGER_WORKING_DIRECTORY_RELEASE        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release"
		VS_DEBUGGER_WORKING_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo"
		VS_DEBUGGER_WORKING_DIRECTORY_MINSIZEREL     "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/MinSizeRel"
	)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/res" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/sfml/bin/$<CONFIG> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		VERBATIM
	)
endif()

if(LINUX OR MINGW)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res 
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/sfml/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>
		VERBATIM
	)
endif()