#pragma once

#include <chrono>
#include <iostream>
#include <string>

// simple performance timer for measuring render times
class PerformanceTimer {
public:
    PerformanceTimer(const std::string& name, bool enabled = true)
        : m_name(name), m_enabled(enabled)
    {
        if (m_enabled) {
            m_start = std::chrono::high_resolution_clock::now();
        }
    }

    ~PerformanceTimer() {
        if (m_enabled) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - m_start);
            std::cout << m_name << ": " << duration.count() << " microseconds ("
                      << (duration.count() / 1000.0f) << " ms)" << std::endl;
        }
    }

private:
    std::string m_name;
    bool m_enabled;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
};
