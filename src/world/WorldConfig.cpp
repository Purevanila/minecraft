#include "world/WorldConfig.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>

// Global configuration instance
WorldConfig g_worldConfig;

bool WorldConfig::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "WorldConfig: Could not open config file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        // Remove whitespace and comments
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        // Check for section headers [section]
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key=value pairs
        size_t equalPos = line.find('=');
        if (equalPos == std::string::npos) continue;
        
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);
        
        // Trim key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        // Apply the setting based on section and key
        applySetting(currentSection, key, value);
    }
    
    file.close();
    validate();
    std::cout << "WorldConfig: Loaded configuration from " << filename << std::endl;
    return true;
}

bool WorldConfig::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cout << "WorldConfig: Could not create config file: " << filename << std::endl;
        return false;
    }
    
    file << "# World Configuration File\n";
    file << "# Generated automatically - feel free to edit!\n\n";
    
    // Rendering settings
    file << "[rendering]\n";
    file << "renderDistance = " << rendering.renderDistance << "\n";
    file << "loadDistance = " << rendering.loadDistance << "\n";
    file << "fogStartDistance = " << rendering.fogStartDistance << "\n";
    file << "fogEndDistance = " << rendering.fogEndDistance << "\n";
    file << "enableFog = " << (rendering.enableFog ? "true" : "false") << "\n";
    file << "enableFrustumCulling = " << (rendering.enableFrustumCulling ? "true" : "false") << "\n";
    file << "maxChunksPerFrame = " << rendering.maxChunksPerFrame << "\n\n";
    
    // Terrain settings
    file << "[terrain]\n";
    file << "seed = " << terrain.seed << "\n";
    file << "seaLevel = " << terrain.seaLevel << "\n";
    file << "minHeight = " << terrain.minHeight << "\n";
    file << "maxHeight = " << terrain.maxHeight << "\n\n";
    
    file << "[terrain.heightNoise]\n";
    file << "frequency = " << terrain.heightNoise.frequency << "\n";
    file << "octaves = " << terrain.heightNoise.octaves << "\n";
    file << "persistence = " << terrain.heightNoise.persistence << "\n";
    file << "lacunarity = " << terrain.heightNoise.lacunarity << "\n";
    file << "amplitude = " << terrain.heightNoise.amplitude << "\n\n";
    
    file << "[terrain.lakes]\n";
    file << "enabled = " << (terrain.lakes.enabled ? "true" : "false") << "\n";
    file << "frequency = " << terrain.lakes.frequency << "\n";
    file << "threshold = " << terrain.lakes.threshold << "\n";
    file << "maxDepth = " << terrain.lakes.maxDepth << "\n\n";
    
    file << "[terrain.plains]\n";
    file << "enabled = " << (terrain.plains.enabled ? "true" : "false") << "\n";
    file << "frequency = " << terrain.plains.frequency << "\n";
    file << "threshold = " << terrain.plains.threshold << "\n";
    file << "flatnessRadius = " << terrain.plains.flatnessRadius << "\n";
    file << "flatnessStrength = " << terrain.plains.flatnessStrength << "\n\n";
    
    file << "[terrain.gravel]\n";
    file << "enabled = " << (terrain.gravel.enabled ? "true" : "false") << "\n";
    file << "frequency = " << terrain.gravel.frequency << "\n";
    file << "density = " << terrain.gravel.density << "\n";
    file << "maxDistance = " << terrain.gravel.maxDistance << "\n";
    file << "edgeBonus = " << terrain.gravel.edgeBonus << "\n\n";
    
    // Tree settings
    file << "[trees]\n";
    file << "enabled = " << (trees.enabled ? "true" : "false") << "\n";
    file << "frequency = " << trees.frequency << "\n";
    file << "threshold = " << trees.threshold << "\n";
    file << "minHeight = " << trees.minHeight << "\n";
    file << "maxHeight = " << trees.maxHeight << "\n";
    file << "minSpacing = " << trees.minSpacing << "\n";
    file << "generateInLakes = " << (trees.generateInLakes ? "true" : "false") << "\n\n";
    
    file << "[trees.leaves]\n";
    file << "enableCrossChunkLeaves = " << (trees.leaves.enableCrossChunkLeaves ? "true" : "false") << "\n";
    file << "minLeavesPerTree = " << trees.leaves.minLeavesPerTree << "\n";
    file << "enablePostProcessing = " << (trees.leaves.enablePostProcessing ? "true" : "false") << "\n\n";
    
    // Performance settings
    file << "[performance]\n";
    file << "enableMultithreadedGeneration = " << (performance.enableMultithreadedGeneration ? "true" : "false") << "\n";
    file << "enableAsyncLoading = " << (performance.enableAsyncLoading ? "true" : "false") << "\n";
    file << "maxMemoryChunks = " << performance.maxMemoryChunks << "\n";
    file << "enableMeshOptimization = " << (performance.enableMeshOptimization ? "true" : "false") << "\n";
    file << "maxChunkUpdatesPerFrame = " << performance.maxChunkUpdatesPerFrame << "\n";
    file << "maxChunksPerFrame = " << performance.maxChunksPerFrame << "\n";
    file << "chunkUpdateDelay = " << performance.chunkUpdateDelay << "\n\n";
    
    // Cloud settings
    file << "[clouds]\n";
    file << "enabled = " << (clouds.enabled ? "true" : "false") << "\n";
    file << "height = " << clouds.height << "\n";
    file << "speed = " << clouds.speed << "\n";
    file << "density = " << clouds.density << "\n";
    file << "updateDistance = " << clouds.updateDistance << "\n";
    file << "gridSize = " << clouds.gridSize << "\n";
    file << "spacing = " << clouds.spacing << "\n";
    file << "layers = " << clouds.layers << "\n";
    file << "layerSpacing = " << clouds.layerSpacing << "\n\n";
    
    // Debug settings
    file << "[debug]\n";
    file << "showChunkBorders = " << (debug.showChunkBorders ? "true" : "false") << "\n";
    file << "showFPS = " << (debug.showFPS ? "true" : "false") << "\n";
    file << "showPlayerPosition = " << (debug.showPlayerPosition ? "true" : "false") << "\n";
    file << "logTreeGeneration = " << (debug.logTreeGeneration ? "true" : "false") << "\n";
    file << "logChunkGeneration = " << (debug.logChunkGeneration ? "true" : "false") << "\n\n";
    
    file.close();
    std::cout << "WorldConfig: Saved configuration to " << filename << std::endl;
    return true;
}

void WorldConfig::resetToDefaults() {
    *this = WorldConfig(); // Reset to default constructor values
}

WorldConfig WorldConfig::getPreset(const std::string& presetName) {
    WorldConfig config;
    
    if (presetName == "performance") {
        // Optimized for performance
        config.rendering.renderDistance = 6;
        config.performance.maxChunksPerFrame = 6;
        config.performance.enableMeshOptimization = true;
        config.trees.leaves.enablePostProcessing = false;
        config.debug.logTreeGeneration = false;
        config.debug.logChunkGeneration = false;
    }
    else if (presetName == "quality") {
        // Optimized for visual quality
        config.rendering.renderDistance = 12;
        config.performance.maxChunksPerFrame = 2;
        config.trees.leaves.enablePostProcessing = true;
        config.rendering.enableFog = true;
        config.performance.enableMeshOptimization = true;
    }
    else if (presetName == "debug") {
        // Optimized for debugging
        config.debug.showChunkBorders = true;
        config.debug.showPlayerPosition = true;
        config.debug.showChunkInfo = true;
        config.debug.logTreeGeneration = true;
        config.debug.logChunkGeneration = true;
        config.rendering.renderDistance = 4;
    }
    else if (presetName == "minimal") {
        // Minimal settings for low-end systems
        config.rendering.renderDistance = 3;
        config.performance.maxChunksPerFrame = 1;
        config.trees.enabled = false;
        config.terrain.lakes.enabled = false;
        config.performance.enableMeshOptimization = false;
    }
    
    return config;
}

void WorldConfig::validate() {
    // Rendering validation
    clampValue(rendering.renderDistance, 1, 32);
    clampValue(rendering.loadDistance, rendering.renderDistance, 64);
    clampValue(rendering.fogStartDistance, 16.0f, 512.0f);
    clampValue(rendering.fogEndDistance, rendering.fogStartDistance + 16.0f, 1024.0f);
    clampValue(rendering.maxChunksPerFrame, 1, 16);
    
    // Terrain validation
    clampValue(terrain.minHeight, 1, 200);
    clampValue(terrain.maxHeight, terrain.minHeight + 10, 255);
    clampValue(terrain.seaLevel, terrain.minHeight, terrain.maxHeight - 5);
    clampValue(terrain.heightNoise.frequency, 0.001, 0.1);
    clampValue(terrain.heightNoise.octaves, 1, 8);
    clampValue(terrain.heightNoise.persistence, 0.1, 1.0);
    clampValue(terrain.heightNoise.lacunarity, 1.5, 4.0);
    clampValue(terrain.heightNoise.amplitude, 5.0, 100.0);
    
    // Lakes validation
    clampValue(terrain.lakes.frequency, 0.001, 0.1);
    clampValue(terrain.lakes.threshold, 0.0, 1.0);
    clampValue(terrain.lakes.maxDepth, 1, 20);
    
    // Plains validation
    clampValue(terrain.plains.frequency, 0.001, 0.1);
    clampValue(terrain.plains.threshold, 0.0, 1.0);
    clampValue(terrain.plains.flatnessRadius, 1, 50);
    clampValue(terrain.plains.flatnessStrength, 0.1, 1.0);
    
    // Gravel validation
    clampValue(terrain.gravel.frequency, 0.001, 0.1);
    clampValue(terrain.gravel.density, 0.0, 1.0);
    clampValue(terrain.gravel.maxDistance, 1.0f, 20.0f);
    clampValue(terrain.gravel.edgeBonus, 0.0, 1.0);
    
    // Tree validation
    clampValue(trees.frequency, 0.001, 0.2);
    clampValue(trees.threshold, 0.0, 1.0);
    clampValue(trees.minHeight, 3, 20);
    clampValue(trees.maxHeight, trees.minHeight, 30);
    clampValue(trees.minSpacing, 2, 20);
    clampValue(trees.leaves.minLeavesPerTree, 1, 50);
    
    // Performance validation
    clampValue(performance.maxMemoryChunks, 50, 1000);
    clampValue(performance.maxChunkUpdatesPerFrame, 1, 10);
    clampValue(performance.chunkUpdateDelay, 0.01f, 1.0f);
}

void WorldConfig::clampValue(int& value, int min, int max) {
    value = std::max(min, std::min(max, value));
}

void WorldConfig::clampValue(float& value, float min, float max) {
    value = std::max(min, std::min(max, value));
}

void WorldConfig::clampValue(double& value, double min, double max) {
    value = std::max(min, std::min(max, value));
}

void WorldConfig::applySetting(const std::string& section, const std::string& key, const std::string& value) {
    try {
        if (section == "rendering") {
            if (key == "renderDistance") rendering.renderDistance = std::stoi(value);
            else if (key == "loadDistance") rendering.loadDistance = std::stoi(value);
            else if (key == "fogStartDistance") rendering.fogStartDistance = std::stof(value);
            else if (key == "fogEndDistance") rendering.fogEndDistance = std::stof(value);
            else if (key == "enableFog") rendering.enableFog = (value == "true");
            else if (key == "enableFrustumCulling") rendering.enableFrustumCulling = (value == "true");
            else if (key == "maxChunksPerFrame") rendering.maxChunksPerFrame = std::stoi(value);
        }
        else if (section == "terrain") {
            if (key == "seed") terrain.seed = std::stoul(value);
            else if (key == "seaLevel") terrain.seaLevel = std::stoi(value);
            else if (key == "minHeight") terrain.minHeight = std::stoi(value);
            else if (key == "maxHeight") terrain.maxHeight = std::stoi(value);
        }
        else if (section == "terrain.heightNoise") {
            if (key == "frequency") terrain.heightNoise.frequency = std::stod(value);
            else if (key == "octaves") terrain.heightNoise.octaves = std::stoi(value);
            else if (key == "persistence") terrain.heightNoise.persistence = std::stod(value);
            else if (key == "lacunarity") terrain.heightNoise.lacunarity = std::stod(value);
            else if (key == "amplitude") terrain.heightNoise.amplitude = std::stod(value);
        }
        else if (section == "terrain.lakes") {
            if (key == "enabled") terrain.lakes.enabled = (value == "true");
            else if (key == "frequency") terrain.lakes.frequency = std::stod(value);
            else if (key == "threshold") terrain.lakes.threshold = std::stod(value);
            else if (key == "maxDepth") terrain.lakes.maxDepth = std::stoi(value);
        }
        else if (section == "terrain.plains") {
            if (key == "enabled") terrain.plains.enabled = (value == "true");
            else if (key == "frequency") terrain.plains.frequency = std::stod(value);
            else if (key == "threshold") terrain.plains.threshold = std::stod(value);
            else if (key == "flatnessRadius") terrain.plains.flatnessRadius = std::stoi(value);
            else if (key == "flatnessStrength") terrain.plains.flatnessStrength = std::stod(value);
        }
        else if (section == "terrain.gravel") {
            if (key == "enabled") terrain.gravel.enabled = (value == "true");
            else if (key == "frequency") terrain.gravel.frequency = std::stod(value);
            else if (key == "density") terrain.gravel.density = std::stod(value);
            else if (key == "maxDistance") terrain.gravel.maxDistance = std::stof(value);
            else if (key == "edgeBonus") terrain.gravel.edgeBonus = std::stod(value);
        }
        else if (section == "trees") {
            if (key == "enabled") trees.enabled = (value == "true");
            else if (key == "frequency") trees.frequency = std::stod(value);
            else if (key == "threshold") trees.threshold = std::stod(value);
            else if (key == "minHeight") trees.minHeight = std::stoi(value);
            else if (key == "maxHeight") trees.maxHeight = std::stoi(value);
            else if (key == "minSpacing") trees.minSpacing = std::stoi(value);
            else if (key == "generateInLakes") trees.generateInLakes = (value == "true");
        }
        else if (section == "trees.leaves") {
            if (key == "enableCrossChunkLeaves") trees.leaves.enableCrossChunkLeaves = (value == "true");
            else if (key == "minLeavesPerTree") trees.leaves.minLeavesPerTree = std::stoi(value);
            else if (key == "enablePostProcessing") trees.leaves.enablePostProcessing = (value == "true");
        }
        else if (section == "performance") {
            if (key == "enableMultithreadedGeneration") performance.enableMultithreadedGeneration = (value == "true");
            else if (key == "enableAsyncLoading") performance.enableAsyncLoading = (value == "true");
            else if (key == "maxMemoryChunks") performance.maxMemoryChunks = std::stoi(value);
            else if (key == "enableMeshOptimization") performance.enableMeshOptimization = (value == "true");
            else if (key == "maxChunkUpdatesPerFrame") performance.maxChunkUpdatesPerFrame = std::stoi(value);
            else if (key == "maxChunksPerFrame") performance.maxChunksPerFrame = std::stoi(value);
            else if (key == "chunkUpdateDelay") performance.chunkUpdateDelay = std::stof(value);
        }
        else if (section == "clouds") {
            if (key == "enabled") clouds.enabled = (value == "true");
            else if (key == "height") clouds.height = std::stof(value);
            else if (key == "speed") clouds.speed = std::stof(value);
            else if (key == "density") clouds.density = std::stof(value);
            else if (key == "updateDistance") clouds.updateDistance = std::stof(value);
            else if (key == "gridSize") clouds.gridSize = std::stoi(value);
            else if (key == "spacing") clouds.spacing = std::stof(value);
            else if (key == "layers") clouds.layers = std::stoi(value);
            else if (key == "layerSpacing") clouds.layerSpacing = std::stof(value);
        }
        else if (section == "debug") {
            if (key == "showChunkBorders") debug.showChunkBorders = (value == "true");
            else if (key == "showFPS") debug.showFPS = (value == "true");
            else if (key == "showPlayerPosition") debug.showPlayerPosition = (value == "true");
            else if (key == "showChunkInfo") debug.showChunkInfo = (value == "true");
            else if (key == "logTreeGeneration") debug.logTreeGeneration = (value == "true");
            else if (key == "logChunkGeneration") debug.logChunkGeneration = (value == "true");
        }
    }
    catch (const std::exception& e) {
        std::cout << "WorldConfig: Error parsing " << section << "." << key << " = " << value << std::endl;
    }
}
