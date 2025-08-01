#pragma once
#include <vector>
#include <glm/glm.hpp>

/**
 * 🌋 Perlin Noise Generator for Natural Terrain
 * 
 * This class generates smooth, natural-looking noise patterns that we use
 * to create realistic terrain heights, cave systems, and ore distributions.
 * Think of it as nature's random number generator!
 */
class PerlinNoise {
public:
    PerlinNoise();
    PerlinNoise(unsigned int seed);
    
    // 🗻 Get height value at a specific 2D coordinate (for terrain height)
    double noise(double x, double y) const;
    
    // 🌊 Get 3D noise value (useful for caves, ore veins, etc.)
    double noise(double x, double y, double z) const;
    
    // 🎛️ Octave noise - combines multiple noise layers for more detail
    double octaveNoise(double x, double y, int octaves, double persistence = 0.5) const;
    double octaveNoise(double x, double y, double z, int octaves, double persistence = 0.5) const;
    
    // 🌄 Advanced terrain generation functions
    double ridgedNoise(double x, double y, int octaves = 4, double persistence = 0.5) const;
    double billowNoise(double x, double y, int octaves = 4, double persistence = 0.5) const;
    double fbm(double x, double y, int octaves = 4, double persistence = 0.5, double lacunarity = 2.0) const;
    double domainWarp(double x, double y, double warpStrength = 0.1) const;
    
    // 🔧 Utility functions for terrain generation
    static double fade(double t);
    static double lerp(double t, double a, double b);
    static double grad(int hash, double x, double y, double z);
    
private:
    std::vector<int> m_permutation;  // Random permutation table
    
    void generatePermutation(unsigned int seed);
};
