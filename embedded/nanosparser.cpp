#include "nanosparser.hpp"
extern "C" {
#include "nanosvg.h"
#include "nanosvgrast.h"
}
#include <cfloat>
#include <cstring>
#include <memory.h>
#include <memory>

SVGParser::SVGParser() {
  handle = (NSVGcontext *)malloc(sizeof(NSVGcontext));
  if (handle) {
    memset(handle, 0, sizeof(NSVGcontext));
  }
}

SVGParser::~SVGParser() {
  if (handle) {
    free(handle);
  }
}

bool SVGParser::open(const char *filename, const char *units, float dpi) {
  auto ret = nsvgOpenFileStream(filename, units, dpi, handle);
  isOpen = ret > 0 ? true : false;
  return isOpen;
}

void SVGParser::parser() {
  handle->bounds = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
  handle->cb_shape = [](NSVGshape *shape, void *self) {
    auto ctx = reinterpret_cast<NSVGcontext *>(self);
    ctx->shapeCount++;
    nsvgUpdateBounds(shape, &ctx->bounds);
  };
  nsvgParseFromFileStream(handle);
}

bool SVGParser::isValid() {
  if (handle && handle->shapeCount > 0) {
    return true;
  }
  return false;
}

void SVGParser::repaser() {
  handle->scaledBounds = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
  handle->cb_shape = [](NSVGshape *shape, void *self) {
    auto ctx = reinterpret_cast<NSVGcontext *>(self);
    nsvgUpdateScaleToViewbox(shape, ctx);
    nsvgUpdateBounds(shape, &ctx->scaledBounds);
  };
  nsvgResetParseFromFileStream(handle);
  debugPrint();
}

SVGParser::Bounds SVGParser::bounds() {
  auto bbox = handle->scaledBounds;
  return {bbox.xmin, bbox.ymin, bbox.xmax - bbox.xmin, bbox.ymax - bbox.ymin};
}

SVGParser::Size SVGParser::size() {
  return {handle->imageWidth, handle->imageHeight};
}

void SVGParser::render(float tx, float ty, float scale, unsigned char *data,
                       int width, int height) {
  auto rasterizer = nsvgCreateRasterizer();

  nsvgInitRasterize(rasterizer, handle, tx, ty, scale, data, width, height,
                    width * 4);

  handle->rasterizer = rasterizer;
  handle->cb_shape = [](NSVGshape *shape, void *self) {
    auto ctx = reinterpret_cast<NSVGcontext *>(self);
    nsvgUpdateScaleToViewbox(shape, ctx);
    nsvgUpdateRasterizer(shape, ctx);
  };
  nsvgResetParseFromFileStream(handle);
  nsvgFinishRasterizer(handle);
  if (rasterizer) {
    nsvgDeleteRasterizer(rasterizer);
  }
}

void SVGParser::close() {
  if (isOpen) {
    nsvgCloseFileStream(handle);
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
  auto bounds = handle->scaledBounds;
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
