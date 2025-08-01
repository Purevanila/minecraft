#pragma once

#include <string>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <memory>
#include <fstream>
#include "utils/ModernCpp.h"

/**
 * Comprehensive performance monitoring system
 * Tracks FPS, memory usage, GPU metrics, and provides detailed profiling
 */
class PerformanceMonitor {
public:
    static PerformanceMonitor& getInstance();
    
    // Core metrics
    struct FrameMetrics {
        double fps = 0.0;
        double frameTimeMs = 0.0;
        double cpuTimeMs = 0.0;
        double gpuTimeMs = 0.0;
        size_t drawCalls = 0;
        size_t triangles = 0;
        size_t verticesRendered = 0;
    };
    
    struct MemoryMetrics {
        size_t totalAllocated = 0;      // Total allocated memory
        size_t peakAllocated = 0;       // Peak memory usage
        size_t currentChunks = 0;       // Number of loaded chunks
        size_t gpuMemoryUsed = 0;       // GPU memory usage (if available)
        size_t textureMemory = 0;       // Memory used by textures
        size_t meshMemory = 0;          // Memory used by meshes
    };
    
    struct WorldMetrics {
        size_t chunksLoaded = 0;
        size_t chunksGenerated = 0;
        size_t chunksRendered = 0;
        size_t blocksVisible = 0;
        double terrainGenTimeMs = 0.0;
        double meshBuildTimeMs = 0.0;
    };
    
    // Update metrics (call once per frame)
    void updateFrame(const FrameMetrics& metrics);
    void updateMemory(const MemoryMetrics& metrics);
    void updateWorld(const WorldMetrics& metrics);
    
    // Profiling functions
    void beginProfile(const std::string& name);
    void endProfile(const std::string& name);
    
    // Automatic RAII profiling
    class ScopedProfiler {
    public:
        explicit ScopedProfiler(std::string name) : m_name(std::move(name)) {
            PerformanceMonitor::getInstance().beginProfile(m_name);
        }
        
        ~ScopedProfiler() {
            PerformanceMonitor::getInstance().endProfile(m_name);
        }
        
    private:
        std::string m_name;
    };
    
    // Get current metrics
    FrameMetrics getCurrentFrameMetrics() const { return m_currentFrame; }
    MemoryMetrics getCurrentMemoryMetrics() const { return m_currentMemory; }
    WorldMetrics getCurrentWorldMetrics() const { return m_currentWorld; }
    
    // Performance analysis
    double getAverageFPS(size_t samples = 60) const;
    double getMinFPS(size_t samples = 60) const;
    double getMaxFPS(size_t samples = 60) const;
    bool isPerformanceGood() const; // Returns true if performance is acceptable
    
    // Logging and reporting
    void enableFileLogging(const std::string& filename);
    void disableFileLogging();
    void logPerformanceReport();
    void printCurrentStats() const;
    
    // Configuration
    void setTargetFPS(double fps) { m_targetFPS = fps; }
    void setWarningThresholds(double minFPS, double maxFrameTime, size_t maxMemoryMB);
    
private:
    PerformanceMonitor() = default;
    
    // Current metrics
    FrameMetrics m_currentFrame;
    MemoryMetrics m_currentMemory;
    WorldMetrics m_currentWorld;
    
    // Historical data
    std::vector<double> m_fpsHistory;
    std::vector<double> m_frameTimeHistory;
    std::vector<size_t> m_memoryHistory;
    
    // Profiling data
    struct ProfileData {
        std::chrono::high_resolution_clock::time_point startTime;
        double totalTime = 0.0;
        size_t callCount = 0;
        double minTime = std::numeric_limits<double>::max();
        double maxTime = 0.0;
    };
    
    std::unordered_map<std::string, ProfileData, Utils::StringHash, std::equal_to<>> m_profileData;
    
    // Configuration
    double m_targetFPS = 60.0;
    double m_warningMinFPS = 30.0;
    double m_warningMaxFrameTime = 33.0; // ms
    size_t m_warningMaxMemoryMB = 1024;
    
    // Logging
    std::unique_ptr<std::ofstream> m_logFile;
    bool m_fileLoggingEnabled = false;
    
    // Utility functions
    void updateHistory();
    void checkPerformanceWarnings() const;
    double calculateAverage(const std::vector<double>& data, size_t samples) const;
};

// Convenient macro for automatic profiling
#define PROFILE_SCOPE(name) PerformanceMonitor::ScopedProfiler CONCAT(_prof_, __COUNTER__)(name)
