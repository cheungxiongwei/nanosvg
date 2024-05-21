#include "nanosparser.hpp"
extern "C" {
#include "nanosvg.h"
#include "nanosvgrast.h"
}
#include <cfloat>
#include <cstring>
#include <memory.h>
#include <memory>

SVGParser::SVGParser() { handle = (NSVGcontext *)malloc(sizeof(NSVGcontext)); }

SVGParser::~SVGParser() {
  if (handle) {
    free(handle);
  }
}

bool SVGParser::open(const char *filename) {
  stream = nsvgOpenFileStream(filename);
  return stream;
}

int SVGParser::parser() {
  if (!stream || !handle) {
    return 0;
  }

  memset(handle, 0, sizeof(NSVGcontext));
  handle->bounds = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
  handle->cb_shape = [](NSVGshape *shape, void *self) {
    auto ctx = reinterpret_cast<NSVGcontext *>(self);
    auto transform = &ctx->transform;
    ctx->shapeCount++;
    nsvgUpdateScaleToViewbox(shape, transform);
    nsvgUpdateBounds(shape, &ctx->bounds);
  };

  nsvgParseFromFileStream(stream, "px", 96, &handle->transform);

  // tx ty sx sy us
  std::println("setp1 {} {} {} {} | {} {} | transform({},{},{},{},{},{})",
               handle->transform.viewMinx, handle->transform.viewMiny,
               handle->transform.viewWidth, handle->transform.viewHeight,
               handle->transform.imageWidth, handle->transform.imageHeight,
               handle->transform.tx, handle->transform.ty, handle->transform.sx,
               handle->transform.sy, handle->transform.us,
               handle->transform.avgs);

  // px boxes
  if constexpr (true) {
    float x, y, width, height;
    auto T = &handle->transform;
    x = T->tx + T->viewMinx;
    y = T->ty + T->viewMiny;
    width = T->sx * T->viewWidth;
    height = T->sy * T->viewHeight;

    std::println("T [x,y,w,h] => [{:.2f},{:.2f},{:.2f},{:.2f}]", x, y, width,
                 height);
  }

  nsvgFastParseFromFileStream(stream, handle);

  debugPrint();

  return 1;
}

void SVGParser::render(float tx, float ty, float scale, unsigned char *data,
                       int width, int height) {
  auto rasterizer = nsvgCreateRasterizer();

  nsvgInitRasterize(rasterizer, handle, tx, ty, scale, data, width, height,
                    width * 4);

  handle->rasterizer = rasterizer;
  handle->cb_shape = [](NSVGshape *shape, void *self) {
    auto ctx = reinterpret_cast<NSVGcontext *>(self);
    nsvgUpdateScaleToViewbox(shape, &ctx->transform);
    nsvgUpdateRasterizer(shape, ctx);
  };
  nsvgFastParseFromFileStream(stream,handle);
  nsvgFinishRasterizer(handle);
  if (rasterizer) {
    nsvgDeleteRasterizer(rasterizer);
  }
}

void SVGParser::close() {
  if (stream) {
    nsvgCloseFileStream(stream);
  }
}

void SVGParser::debugPrint() {
  if (!handle) {
    return;
  }

  typedef struct preview_t {
    float x, y, width, height;
  } preview_t;

  auto ctx = handle;
  auto bounds = handle->bounds;
  preview_t bbox;
  bbox.x = bounds.xmin;
  bbox.y = bounds.ymin;
  bbox.width = bounds.xmax - bounds.xmin;
  bbox.height = bounds.ymax - bounds.ymin;

  std::println("px [{}, {}, {}, {}] shape:{}", bbox.x, bbox.y, bbox.width,
               bbox.height, ctx->shapeCount);

  auto mm = [](float value, float dpi = 96.f) -> auto {
    return value / dpi * 25.4f;
  };

  std::println("mm [{}, {}, {}, {}] shape:{}", mm(bbox.x), mm(bbox.y),
               mm(bbox.width), mm(bbox.height), ctx->shapeCount);
}
