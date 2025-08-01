#include "world/PerlinNoise.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <numeric>

PerlinNoise::PerlinNoise() {
    generatePermutation(12345); // Default seed
}

PerlinNoise::PerlinNoise(unsigned int seed) {
    generatePermutation(seed);
}

void PerlinNoise::generatePermutation(unsigned int seed) {
    // Create permutation table with values 0-255
    m_permutation.resize(512);
    
    // Fill first 256 with sequential values
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    
    // Shuffle using the provided seed for deterministic but randomized results
    std::default_random_engine generator(seed);
    std::shuffle(p.begin(), p.end(), generator);
    
    // Duplicate the permutation to avoid overflow and provide seamless wrapping
    for (int i = 0; i < 256; ++i) {
        m_permutation[i] = p[i];
        m_permutation[i + 256] = p[i];
    }
}

double PerlinNoise::noise(double x, double y) const {
    return noise(x, y, 0.0);
}

double PerlinNoise::noise(double x, double y, double z) const {
    // 🔍 Find the unit cube that contains the point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;
    
    // 📍 Find relative position within the cube
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);
    
    // 🌊 Compute fade curves for smoothing
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);
    
    // 🎲 Hash coordinates of the 8 cube corners with proper bounds checking
    int A = m_permutation[X] + Y;
    int AA = m_permutation[A & 255] + Z;
    int AB = m_permutation[(A + 1) & 255] + Z;
    int B = m_permutation[(X + 1) & 255] + Y;
    int BA = m_permutation[B & 255] + Z;
    int BB = m_permutation[(B + 1) & 255] + Z;
    
    // 🌈 Interpolate between the 8 gradient values with proper indexing
    return lerp(w, 
        lerp(v, 
            lerp(u, grad(m_permutation[AA & 255], x, y, z),
                    grad(m_permutation[BA & 255], x - 1, y, z)),
            lerp(u, grad(m_permutation[AB & 255], x, y - 1, z),
                    grad(m_permutation[BB & 255], x - 1, y - 1, z))),
        lerp(v, 
            lerp(u, grad(m_permutation[(AA + 1) & 255], x, y, z - 1),
                    grad(m_permutation[(BA + 1) & 255], x - 1, y, z - 1)),
            lerp(u, grad(m_permutation[(AB + 1) & 255], x, y - 1, z - 1),
                    grad(m_permutation[(BB + 1) & 255], x - 1, y - 1, z - 1))));
}

double PerlinNoise::octaveNoise(double x, double y, int octaves, double persistence) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    // Clamp octaves to reasonable range to prevent performance issues
    octaves = std::max(1, std::min(octaves, 8));
    
    // 🎵 Combine multiple octaves for detailed terrain
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * frequency, y * frequency) * amplitude;
        
        maxValue += amplitude;
        amplitude *= persistence;  // Each octave has less influence
        frequency *= 2.0;          // Each octave has higher frequency
    }
    
    // Normalize to maintain consistent range regardless of octave count
    return maxValue > 0.0 ? total / maxValue : 0.0;
}

double PerlinNoise::octaveNoise(double x, double y, double z, int octaves, double persistence) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    // Clamp octaves to reasonable range to prevent performance issues
    octaves = std::max(1, std::min(octaves, 8));
    
    // 🎵 3D octave noise for caves and ore generation
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
        
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    // Normalize to maintain consistent range regardless of octave count
    return maxValue > 0.0 ? total / maxValue : 0.0;
}

// 🌊 Improved fade function for smoother interpolation
double PerlinNoise::fade(double t) {
    // Enhanced fade curve: 6t^5 - 15t^4 + 10t^3 (Ken Perlin's improved version)
    // This provides C2 continuity for smoother terrain
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// 🔗 Linear interpolation
double PerlinNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

// 🎯 Enhanced gradient function with better directional vectors
double PerlinNoise::grad(int hash, double x, double y, double z) {
    // Use improved gradients with better distribution
    // Based on Ken Perlin's reference implementation with 12 edge vectors of a cube
    static const double gradients[32][3] = {
        // 12 edges of a cube
        {1,1,0}, {-1,1,0}, {1,-1,0}, {-1,-1,0},
        {1,0,1}, {-1,0,1}, {1,0,-1}, {-1,0,-1},
        {0,1,1}, {0,-1,1}, {0,1,-1}, {0,-1,-1},
        // 8 corners of a cube (additional variety)
        {1,1,1}, {-1,1,1}, {1,-1,1}, {-1,-1,1},
        {1,1,-1}, {-1,1,-1}, {1,-1,-1}, {-1,-1,-1},
        // 12 more for better distribution
        {0,1,0}, {0,-1,0}, {1,0,0}, {-1,0,0},
        {0,0,1}, {0,0,-1}, {0.7071,0.7071,0}, {-0.7071,0.7071,0},
        {0.7071,-0.7071,0}, {-0.7071,-0.7071,0}, {0.7071,0,0.7071}, {-0.7071,0,0.7071}
    };
    
    int index = hash & 31;  // Use all 32 gradients
    const double* g = gradients[index];
    return g[0] * x + g[1] * y + g[2] * z;
}

// 🏔️ Ridged noise - creates sharp mountain ridges and canyons
double PerlinNoise::ridgedNoise(double x, double y, int octaves, double persistence) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    octaves = std::max(1, std::min(octaves, 8));
    
    for (int i = 0; i < octaves; ++i) {
        // Take absolute value and invert for ridged effect
        double noiseValue = std::abs(noise(x * frequency, y * frequency));
        noiseValue = 1.0 - noiseValue;  // Invert for ridges
        noiseValue = noiseValue * noiseValue;  // Square for sharper ridges
        
        total += noiseValue * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    return maxValue > 0.0 ? total / maxValue : 0.0;
}

// 🌊 Billow noise - creates puffy cloud-like formations
double PerlinNoise::billowNoise(double x, double y, int octaves, double persistence) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    octaves = std::max(1, std::min(octaves, 8));
    
    for (int i = 0; i < octaves; ++i) {
        // Take absolute value for billow effect
        double noiseValue = std::abs(noise(x * frequency, y * frequency));
        
        total += noiseValue * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    
    return maxValue > 0.0 ? total / maxValue : 0.0;
}

// 🌍 Fractal Brownian Motion - more sophisticated octave combination
double PerlinNoise::fbm(double x, double y, int octaves, double persistence, double lacunarity) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;
    
    octaves = std::max(1, std::min(octaves, 8));
    
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * frequency, y * frequency) * amplitude;
        
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;  // Use custom lacunarity instead of fixed 2.0
    }
    
    return maxValue > 0.0 ? total / maxValue : 0.0;
}

// 🌀 Domain warping - distorts coordinate space for more organic shapes
double PerlinNoise::domainWarp(double x, double y, double warpStrength) const {
    // Sample noise at offset positions to warp the domain
    double warpX = noise(x + 100.0, y + 200.0) * warpStrength;
    double warpY = noise(x + 300.0, y + 400.0) * warpStrength;
    
    // Return noise sampled at warped coordinates
    return noise(x + warpX, y + warpY);
}
