# Minecraft Clone

A Minecraft-inspired voxel game written in C++ using OpenGL.

## Features

- 3D voxel-based world
- Chunk-based world generation
- Basic block physics
- Player movement and camera controls
- Texture rendering
- Basic lighting

## Dependencies

- OpenGL 3.3+
- GLFW3
- GLM
- GLAD (for OpenGL loading)

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Running

```bash
./MinecraftClone
```

## Project Structure

- `src/` - Source code
  - `core/` - Core game systems
  - `engine/` - Rendering and input engine
  - `world/` - World generation and management
  - `blocks/` - Block definitions and logic
  - `entities/` - Game entities (player, mobs)
  - `ui/` - User interface
  - `network/` - Multiplayer networking
  - `utils/` - Utility functions
- `include/` - Header files
- `assets/` - Game assets (textures, shaders, sounds)
- `lib/` - Third-party libraries
- `tests/` - Unit tests
- `docs/` - Documentation
