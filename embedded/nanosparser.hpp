#ifndef NANOSVGINJECT_H
#define NANOSVGINJECT_H

#include <print>

class SVGParser {
public:
  SVGParser();

  ~SVGParser();

  bool open(const char *filename);

  int parser();

  void render(float tx, float ty, float scale, unsigned char *data, int width,
              int height);

  void close();

  void debugPrint();

  // source ∈ target
  static float calcRasterizerScale(float sw, float sh, float tw, float th) {
    sw = tw / sw;
    sh = th / sh;
    return sw < sh ? sw : sh;
  };

private:
  struct NSVGcontext *handle{nullptr};
  struct NSVGstream *stream{nullptr};
};

#endif // NANOSVGINJECT_H
