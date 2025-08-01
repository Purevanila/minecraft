#pragma once
#include <string>

/**
 * World Configuration System
 * Centralized configuration for all world generation and rendering parameters
 */
struct WorldConfig {
    // RENDERING SETTINGS
    struct Rendering {
        int renderDistance = 8;         // Chunks to render in each direction
        int loadDistance = 10;          // Chunks to keep loaded (should be >= renderDistance)
        float fogStartDistance = 64.0f; // Distance where fog starts
        float fogEndDistance = 128.0f;  // Distance where fog is completely opaque
        bool enableFog = true;          // Whether to use fog for distant chunks
        bool enableFrustumCulling = true; // Cull chunks outside view frustum
        int maxChunksPerFrame = 4;      // Max chunks to generate per frame (for lag prevention)
    } rendering;
    
    // TERRAIN GENERATION SETTINGS
    struct Terrain {
        unsigned int seed = 12345;      // World seed for deterministic generation
        int seaLevel = 32;              // Water level for oceans/lakes
        int minHeight = 10;             // Minimum terrain height
        int maxHeight = 80;             // Maximum terrain height
        
        // Noise settings for terrain height
        struct HeightNoise {
            double frequency = 0.01;    // How zoomed in the noise is (smaller = larger features)
            int octaves = 4;            // Number of noise layers
            double persistence = 0.5;   // How much each octave contributes
            double lacunarity = 2.0;    // Frequency multiplier between octaves
            double amplitude = 30.0;    // Height variation amount
        } heightNoise;
        
        // Noise settings for biome/terrain variation
        struct BiomeNoise {
            double frequency = 0.005;   // Very large scale biome features
            double threshold = 0.3;     // Controls biome distribution
        } biomeNoise;
        
        // Lake generation
        struct Lakes {
            bool enabled = true;
            double frequency = 0.02;    // How often lakes appear
            double threshold = 0.6;     // Higher = fewer lakes
            int maxDepth = 8;           // Maximum lake depth
        } lakes;
        
        // Plains generation
        struct Plains {
            bool enabled = true;
            double frequency = 0.008;   // How often plains appear
            double threshold = 0.4;     // Higher = fewer plains
            int flatnessRadius = 8;     // How large the flat areas are
            double flatnessStrength = 0.8; // How flat they are (0.0 = normal terrain, 1.0 = perfectly flat)
        } plains;
        
        // Gravel generation near water
        struct Gravel {
            bool enabled = true;
            double frequency = 0.08;    // Frequency of gravel patches
            double density = 0.6;       // How dense gravel patches are
            int maxDistance = 4;        // Maximum distance from water to generate gravel
            double edgeBonus = 0.5;     // Extra gravel probability right at water edge
        } gravel;
    } terrain;
    
    // TREE GENERATION SETTINGS
    struct Trees {
        bool enabled = true;
        double frequency = 0.05;        // How dense trees are
        double threshold = 0.3;         // Higher = fewer trees
        int minHeight = 4;              // Minimum tree height
        int maxHeight = 7;              // Maximum tree height
        int minSpacing = 5;             // Minimum blocks between trees
        bool generateInLakes = false;   // Whether trees can spawn in lake areas
        
        // Leaf generation settings
        struct Leaves {
            bool enableCrossChunkLeaves = false; // Whether to try placing leaves across chunk boundaries
            int minLeavesPerTree = 8;   // Minimum leaves required for a "complete" tree
            bool enablePostProcessing = true;   // Fix incomplete trees after generation
        } leaves;
    } trees;
    
    // PERFORMANCE SETTINGS
    struct Performance {
        bool enableMultithreadedGeneration = false; // Threaded chunk generation (experimental)
        bool enableAsyncLoading = true;     // Load chunks asynchronously
        int maxMemoryChunks = 200;          // Max chunks to keep in memory
        bool enableMeshOptimization = true; // Optimize mesh generation
        bool enableGreedyMeshing = false;   // Advanced mesh optimization (experimental)
        
        // Chunk update settings
        int maxChunkUpdatesPerFrame = 2;    // Max chunk mesh updates per frame
        int maxChunksPerFrame = 4;          // Max chunks to generate per frame
        float chunkUpdateDelay = 0.1f;      // Delay between chunk updates (seconds)
    } performance;
    
    // CLOUD SETTINGS
    struct Clouds {
        bool enabled = true;                // Whether to render clouds
        float height = 80.0f;               // Height of cloud layer
        float speed = 0.01f;                // Cloud movement speed
        float density = 0.5f;               // Cloud density (0.0 - 1.0)
        float updateDistance = 64.0f;       // Regenerate clouds when player moves this distance
        int gridSize = 32;                  // Size of cloud grid
        float spacing = 8.0f;               // Spacing between cloud positions
        int layers = 6;                     // Number of cloud layers
        float layerSpacing = 2.5f;          // Spacing between cloud layers
    } clouds;
    
    // GAMEPLAY SETTINGS
    struct Gameplay {
        float playerWalkSpeed = 5.0f;       // Blocks per second
        float playerRunSpeed = 8.0f;        // Blocks per second when running
        float jumpHeight = 1.2f;            // Jump height in blocks
        float gravity = 9.8f;               // Gravity acceleration
        bool enableFlying = true;           // Allow player flying
        float flySpeed = 15.0f;             // Flying speed
    } gameplay;
    
    // DEBUG SETTINGS
    struct Debug {
        bool showChunkBorders = false;      // Render chunk boundary lines
        bool showFPS = true;                // Display FPS counter
        bool showPlayerPosition = true;     // Display player coordinates
        bool showChunkInfo = false;         // Display chunk generation info
        bool enableWireframe = false;       // Render in wireframe mode
        bool logTreeGeneration = false;     // Log tree generation details
        bool logChunkGeneration = false;    // Log chunk generation details
    } debug;
    
    // LIGHTING SETTINGS (for future use)
    struct Lighting {
        bool enableDynamicLighting = false; // Dynamic lighting system
        bool enableShadows = false;         // Shadow rendering
        float ambientLight = 0.3f;          // Ambient light level (0-1)
        float sunBrightness = 1.0f;         // Sun brightness (0-1)
    } lighting;
    
    // METHODS
    
    /**
     * Load configuration from a file
     * Returns true if successful, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * Save configuration to a file
     * Returns true if successful, false otherwise
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * Reset all settings to defaults
     */
    void resetToDefaults();
    
    /**
     * Get a preset configuration
     */
    static WorldConfig getPreset(const std::string& presetName);
    
    /**
     * Validate configuration values and clamp to reasonable ranges
     */
    void validate();
    
private:
    void clampValue(int& value, int min, int max);
    void clampValue(float& value, float min, float max);
    void clampValue(double& value, double min, double max);
    void applySetting(const std::string& section, const std::string& key, const std::string& value);
};

/**
 * Global configuration instance
 */
extern WorldConfig g_worldConfig;
