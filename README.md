# Voxel Engine Editor

A lightweight C++ Voxel Engine Editor using OpenGL, GLFW, and ImGui.

## Features

- **Chunk Management**: Create and manage voxel chunks.
- **Procedural Generation**: Generate Cubes, Spheres, and Cylinders.
- **Model Import**: Import `.obj` files and voxelize them.
- **Physics**: Basic rigid body physics with gravity.
- **Editor UI**: Inspect and modify object properties, voxel size, and transforms.

## Controls

- **W, A, S, D**: Move Camera
- **Right-Click (Hold)**: Rotate Camera
- **Left-Shift**: Sprint
- **Left-Click**: Select Object in Scene
- **ESC**: Close Application

## Building

### Prerequisites

- CMake (3.10+)
- C++ Compiler (C++17 support)
- OpenGL 4.6 compatible drivers

### Instructions

1.  Clone the repository.
2.  Configure with CMake:
    ```bash
    cmake -S . -B build
    ```
3.  Build:
    ```bash
    cmake --build build
    ```
4.  Run:
    ```bash
    ./build/VoxelEngine
    ```
    (Note: ensure `shaders/` and `models/` folders are accessible relative to the executable, or run from root if configured.)

## Architecture

The project has been refactored for better modularity:

- **Core Engine**: `Application` class handles the main loop.
- **Rendering**: `Chunk` handles mesh generation and rendering.
- **UI**: `GUI` class handles ImGui integration.
- **Loaders**: `ModelLoader` handles external file formats.
