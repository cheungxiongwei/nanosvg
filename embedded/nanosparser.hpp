#ifndef NANOSVGINJECT_H
#define NANOSVGINJECT_H

#include <print>

#if 1
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
#endif

class SVGParser {
public:
  struct Bounds {
    float x, y, width, height;
  };

  struct Size {
    float width, height;
  };

  SVGParser();

  ~SVGParser();

  bool open(const char *filename, const char *units, float dpi);

  void parser();

  bool isValid();

  void repaser();

  Bounds bounds();

  Size size();

  void render(float tx, float ty, float scale, unsigned char *data, int width,
              int height);

  void close();

  void debugPrint();

  // source ∈ target
  static float CalcRasterizerScale(float sw, float sh, float tw, float th) {
    sw = tw / sw;
    sh = th / sh;
    return sw < sh ? sw : sh;
  };

private:
  struct NSVGcontext *handle{nullptr};
  bool isOpen{false};
};

#endif // NANOSVGINJECT_H
