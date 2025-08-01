#pragma once
#include <chrono>
#include <string>
#include <unordered_map>
#include <iostream>

/**
 * Simple profiling system to identify performance bottlenecks
 */
class Profiler {
public:
    static Profiler& getInstance() {
        static Profiler instance;
        return instance;
    }
    
    void startTimer(const std::string& name) {
        m_timers[name] = std::chrono::high_resolution_clock::now();
    }
    
    void endTimer(const std::string& name) {
        auto it = m_timers.find(name);
        if (it != m_timers.end()) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - it->second);
            m_results[name] += duration.count();
            m_counts[name]++;
            m_timers.erase(it);
        }
    }
    
    void printResults() const {
        std::cout << "\n=== Performance Profile ===" << std::endl;
        for (const auto& [name, totalTime] : m_results) {
            int count = m_counts.at(name);
            double avgTime = static_cast<double>(totalTime) / count;
            std::cout << name << ": " << avgTime << "μs avg (" << count << " calls)" << std::endl;
        }
        std::cout << "==========================\n" << std::endl;
    }
    
    void reset() {
        m_timers.clear();
        m_results.clear();
        m_counts.clear();
    }
    
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> m_timers;
    std::unordered_map<std::string, long long> m_results;
    std::unordered_map<std::string, int> m_counts;
};

// RAII timer for automatic profiling
class ScopedTimer {
public:
    ScopedTimer(const std::string& name) : m_name(name) {
        Profiler::getInstance().startTimer(m_name);
    }
    
    ~ScopedTimer() {
        Profiler::getInstance().endTimer(m_name);
    }
    
private:
    std::string m_name;
};

#define PROFILE(name) ScopedTimer timer(name)
