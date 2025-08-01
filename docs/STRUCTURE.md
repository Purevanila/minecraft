# Project Structure Documentation

## Overview
This is a Minecraft-inspired voxel game built with C++ and OpenGL.

## Directory Structure

### `/src` - Source Code
- **`core/`** - Core game systems and main game loop
  - `Game.cpp/h` - Main game class, initialization, and game loop
  - `GameState.cpp/h` - Game state management (menu, playing, paused)
  - `Settings.cpp/h` - Game settings and configuration

- **`engine/`** - Engine components
  - **`graphics/`** - Rendering system
    - `Renderer.cpp/h` - Main rendering system
    - `Shader.cpp/h` - Shader loading and management
    - `Texture.cpp/h` - Texture loading and management
    - `Mesh.cpp/h` - Mesh data structures
    - `Window.cpp/h` - Window management
    - `Camera.cpp/h` - Camera system
  - **`input/`** - Input handling
    - `InputManager.cpp/h` - Input event handling
    - `KeyboardInput.cpp/h` - Keyboard input processing
    - `MouseInput.cpp/h` - Mouse input processing
  - **`audio/`** - Audio system
    - `AudioManager.cpp/h` - Audio management
    - `Sound.cpp/h` - Sound effects
    - `Music.cpp/h` - Background music

- **`world/`** - World generation and management
  - `World.cpp/h` - Main world management
  - `Chunk.cpp/h` - Chunk data structure and rendering
  - `ChunkManager.cpp/h` - Chunk loading/unloading
  - **`generation/`** - World generation algorithms
    - `TerrainGenerator.cpp/h` - Terrain generation
    - `NoiseGenerator.cpp/h` - Perlin noise implementation
    - `BiomeGenerator.cpp/h` - Biome generation
    - `StructureGenerator.cpp/h` - Structure generation (trees, caves)

- **`blocks/`** - Block system
  - `Block.cpp/h` - Base block class
  - `BlockRegistry.cpp/h` - Block type registry
  - `BlockMesh.cpp/h` - Block mesh generation
  - `BlockPhysics.cpp/h` - Block physics

- **`entities/`** - Game entities
  - `Entity.cpp/h` - Base entity class
  - `Player.cpp/h` - Player entity
  - `Mob.cpp/h` - Mobile entities
  - `Item.cpp/h` - Item entities
  - `EntityManager.cpp/h` - Entity management

- **`ui/`** - User interface
  - `UIManager.cpp/h` - UI system management
  - `Menu.cpp/h` - Menu system
  - `HUD.cpp/h` - Heads-up display
  - `Inventory.cpp/h` - Inventory interface

- **`network/`** - Multiplayer networking
  - `NetworkManager.cpp/h` - Network management
  - `Client.cpp/h` - Client networking
  - `Server.cpp/h` - Server networking
  - `Packet.cpp/h` - Network packet handling

- **`utils/`** - Utility functions
  - `Math.cpp/h` - Mathematical utilities
  - `FileUtils.cpp/h` - File I/O utilities
  - `Timer.cpp/h` - Timing utilities
  - `Logger.cpp/h` - Logging system

### `/include` - Header Files
Mirror structure of `/src` for header files.

### `/assets` - Game Assets
- **`textures/`** - Texture files
  - `blocks/` - Block textures
  - `entities/` - Entity textures
  - `ui/` - UI textures
- **`shaders/`** - GLSL shader files
- **`sounds/`** - Audio files
- **`models/`** - 3D model files

### `/lib` - Third-party Libraries
External dependencies and libraries.

### `/tests` - Unit Tests
Test files for various components.

### `/build` - Build Output
Generated build files and executables.

### `/docs` - Documentation
Additional documentation and design documents.
