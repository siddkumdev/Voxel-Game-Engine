cmake_minimum_required(VERSION 3.10)
project(VoxelEngine LANGUAGES C CXX)

# --- ADD THIS SECTION ---
if(MSVC)
    # For Visual Studio: /O2 is optimization, /Ob2 is inline function expansion
    add_compile_options(/O2 /Ob2)
else()
    # For GCC/Clang: -O3 is max optimization
    add_compile_options(-O3)
endif()
# ------------------------

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Define Paths
set(DEP_DIR "${CMAKE_SOURCE_DIR}/dependencies")

# 2. Include Directories (Replacement for your -I flags)
include_directories(
    "src"
    "${DEP_DIR}/GLAD/include"
    "${DEP_DIR}/GLFW/include"
    "${DEP_DIR}/GLM"
    "${DEP_DIR}/IMGUI"
    "${DEP_DIR}/IMGUI/backends"
)

# 3. Find Source Files
file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp" "src/*.c")
file(GLOB IMGUI_SOURCES "${DEP_DIR}/IMGUI/*.cpp")
set(IMGUI_BACKENDS
    "${DEP_DIR}/IMGUI/backends/imgui_impl_glfw.cpp"
    "${DEP_DIR}/IMGUI/backends/imgui_impl_opengl3.cpp"
)

# 4. Define the Executable
add_executable(VoxelEngine 
    ${ENGINE_SOURCES} 
    ${IMGUI_SOURCES} 
    ${IMGUI_BACKENDS}
)

# 5. Link Libraries (Replacement for your -L and -l flags)
target_link_directories(VoxelEngine PRIVATE "${DEP_DIR}/GLFW/libs")
target_link_libraries(VoxelEngine 
    glfw3 
    opengl32 
    gdi32
)

# Create a folder in the build directory for the shaders
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")

# Copy shaders whenever they change
add_custom_command(
    TARGET VoxelEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/src/shaders"
            "${CMAKE_BINARY_DIR}/shaders"
    COMMENT "Syncing shaders to build directory..."
)