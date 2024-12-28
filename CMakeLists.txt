cmake_minimum_required(VERSION 3.0)
project(final)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Add external libraries
add_subdirectory(external)

# Include directories
include_directories(
		external/glfw-3.1.2/include/
		external/glm-0.9.7.1/
		external/glad-opengl-3.3/include/
		external/tinygltf-2.9.3/
		external/
		final/
)

# Define the executable
add_executable(opengl_futuristic
		final/main.cpp
		final/render/shader.cpp
		final/skybox.cpp
		final/skybox.h
		final/asset.cpp
		final/asset.h
		final/cube.cpp
		final/cube.h
		final/texture.cpp
		final/texture.h
		final/ground.cpp
		final/ground.h
		final/lighting.cpp
		final/lighting.h
        final/baseObject.h
		final/baseObject.cpp
		final/instance.h
		final/MyBot.cpp
)

# Link libraries to the executable
target_link_libraries(opengl_futuristic
		${OPENGL_LIBRARY}
		glfw
		glad
)
