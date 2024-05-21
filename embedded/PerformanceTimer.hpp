#pragma once

#include <chrono>
class PerformanceTimer {
public:
  PerformanceTimer() : m_start(), m_end() {}

  void start() { m_start = std::chrono::high_resolution_clock::now(); }

  void stop() { m_end = std::chrono::high_resolution_clock::now(); }

  float elapsedMilliseconds() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_end -
                                                                 m_start)
        .count();
  }

  float elapsedSeconds() const {
    return std::chrono::duration_cast<std::chrono::duration<float>>(m_end -
                                                                    m_start)
        .count();
  }

private:
  std::chrono::high_resolution_clock::time_point m_start;
  std::chrono::high_resolution_clock::time_point m_end;
};
