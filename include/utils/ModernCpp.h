#pragma once

#include <memory>
#include <utility>
#include <type_traits>
#include <chrono>
#include <functional>

namespace Utils {

    // RAII scope guard for automatic cleanup
    template<typename F>
    class ScopeGuard {
    public:
        explicit ScopeGuard(F&& func) : m_func(std::forward<F>(func)), m_active(true) {}
        
        ~ScopeGuard() {
            if (m_active) {
                m_func();
            }
        }
        
        // Move constructor
        ScopeGuard(ScopeGuard&& other) noexcept 
            : m_func(std::move(other.m_func)), m_active(other.m_active) {
            other.m_active = false;
        }
        
        // Non-copyable
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
        ScopeGuard& operator=(ScopeGuard&&) = delete;
        
        void dismiss() { m_active = false; }
        
    private:
        F m_func;
        bool m_active;
    };
    
    // Helper function to create scope guards
    template<typename F>
    auto makeScopeGuard(F&& func) {
        return ScopeGuard<std::decay_t<F>>(std::forward<F>(func));
    }
    
    // Fast hash combine function for creating composite hash values
    template<typename T>
    constexpr void hashCombine(std::size_t& seed, const T& value) noexcept {
        std::hash<T> hasher;
        seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    
    // High-resolution timer for performance measurements
    class Timer {
    public:
        Timer() : m_start(std::chrono::high_resolution_clock::now()) {}
        
        void reset() {
            m_start = std::chrono::high_resolution_clock::now();
        }
        
        double elapsedMs() const {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start);
            return duration.count() / 1000.0;
        }
        
        double elapsedUs() const {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start);
            return static_cast<double>(duration.count());
        }
        
    private:
        std::chrono::high_resolution_clock::time_point m_start;
    };
    
    // Efficient string hashing for unordered containers
    struct StringHash {
        using is_transparent = void;  // Enable heterogeneous lookup
        
        std::size_t operator()(const std::string& str) const noexcept {
            return std::hash<std::string>{}(str);
        }
        
        std::size_t operator()(const char* str) const noexcept {
            return std::hash<std::string_view>{}(str);
        }
        
        std::size_t operator()(std::string_view str) const noexcept {
            return std::hash<std::string_view>{}(str);
        }
    };
    
    // Non-owning span-like class for array views (C++20 alternative)
    template<typename T>
    class Span {
    public:
        constexpr Span() noexcept : m_data(nullptr), m_size(0) {}
        constexpr Span(T* data, std::size_t size) noexcept : m_data(data), m_size(size) {}
        
        template<std::size_t N>
        constexpr Span(T (&array)[N]) noexcept : m_data(array), m_size(N) {}
        
        template<typename Container>
        constexpr Span(Container& container) noexcept 
            : m_data(container.data()), m_size(container.size()) {}
        
        constexpr T* data() const noexcept { return m_data; }
        constexpr std::size_t size() const noexcept { return m_size; }
        constexpr bool empty() const noexcept { return m_size == 0; }
        
        constexpr T& operator[](std::size_t index) const noexcept { return m_data[index]; }
        constexpr T* begin() const noexcept { return m_data; }
        constexpr T* end() const noexcept { return m_data + m_size; }
        
    private:
        T* m_data;
        std::size_t m_size;
    };
    
    // Deduction guides for Span
    template<typename T, std::size_t N>
    Span(T (&)[N]) -> Span<T>;
    
    template<typename Container>
    Span(Container&) -> Span<typename Container::value_type>;
    
} // namespace Utils

// Convenient macros for RAII cleanup
#define SCOPE_EXIT(code) auto CONCAT(_scope_guard_, __COUNTER__) = Utils::makeScopeGuard([&]() { code; })
#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)
