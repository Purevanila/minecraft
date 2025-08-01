#pragma once

#include <chrono>
#include <deque>
#include <numeric>

/**
 * High-precision game loop with adaptive timing
 * Features frame limiting, delta smoothing, and performance monitoring
 */
class GameLoop {
public:
    explicit GameLoop(double targetFPS = 60.0, size_t smoothingSamples = 10);
    
    // Start the main game loop
    template<typename UpdateFunc, typename RenderFunc>
    void run(UpdateFunc&& update, RenderFunc&& render) {
        auto lastTime = std::chrono::high_resolution_clock::now();
        
        while (m_running) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            double deltaTime = std::chrono::duration<double>(currentTime - lastTime).count();
            lastTime = currentTime;
            
            // Clamp delta time to prevent large jumps
            deltaTime = std::min(deltaTime, m_maxDeltaTime);
            
            // Smooth delta time for more stable gameplay
            updateSmoothDelta(deltaTime);
            
            // Update game logic
            update(m_smoothDelta);
            
            // Render frame
            render();
            
            // Frame limiting
            if (m_frameLimit > 0) {
                limitFrameRate(currentTime);
            }
            
            // Update statistics
            updateStats(deltaTime);
        }
    }
    
    // Control functions
    void stop() { m_running = false; }
    bool isRunning() const { return m_running; }
    
    // Configuration
    void setTargetFPS(double fps) { 
        m_targetFPS = fps; 
        m_frameTime = 1.0 / fps;
        m_frameLimit = fps > 0 ? std::chrono::duration<double>(m_frameTime) : std::chrono::duration<double>::zero();
    }
    
    void setVSync(bool enabled) { m_vsync = enabled; }
    void setMaxDeltaTime(double maxDelta) { m_maxDeltaTime = maxDelta; }
    
    // Performance metrics
    double getCurrentFPS() const { return m_currentFPS; }
    double getAverageFPS() const { return m_averageFPS; }
    double getSmoothDelta() const { return m_smoothDelta; }
    double getFrameTime() const { return m_frameTime; }
    
    // Performance statistics
    struct PerformanceStats {
        double currentFPS = 0.0;
        double averageFPS = 0.0;
        double minFPS = 0.0;
        double maxFPS = 0.0;
        double frameTimeMs = 0.0;
        double smoothDelta = 0.0;
        size_t totalFrames = 0;
    };
    
    PerformanceStats getStats() const;
    
private:
    bool m_running = true;
    double m_targetFPS;
    double m_frameTime;
    double m_maxDeltaTime = 0.05; // Cap at 20 FPS minimum
    bool m_vsync = false;
    
    // Timing
    std::chrono::duration<double> m_frameLimit;
    
    // Delta time smoothing
    std::deque<double> m_deltaHistory;
    size_t m_smoothingSamples;
    double m_smoothDelta = 0.0;
    
    // Performance tracking
    double m_currentFPS = 0.0;
    double m_averageFPS = 0.0;
    double m_minFPS = std::numeric_limits<double>::max();
    double m_maxFPS = 0.0;
    size_t m_frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_statsStartTime;
    
    void updateSmoothDelta(double deltaTime);
    void limitFrameRate(const std::chrono::time_point<std::chrono::high_resolution_clock>& frameStart);
    void updateStats(double deltaTime);
};
