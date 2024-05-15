#include "nanosparser.hpp"
#include "nanosvgrast.h"
#include <algorithm>
#include <cfloat>

#include <stdio.h>

#include "bezier.h"

int main() {
  constexpr auto svg = R"(C:\Users\cheungxiongwei\Pictures\tig.svg)";

  // Load SVG
  NSVGimage *image;
  image = nsvgParseFromFile(svg, "px", 96);
  printf("size: %f x %f\n", image->width, image->height);
  // Use...
  float bounds[4] = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
  ;
  for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
    bounds[0] = std::min(bounds[0], shape->bounds[0]);
    bounds[1] = std::min(bounds[1], shape->bounds[1]);
    bounds[2] = std::max(bounds[2], shape->bounds[2]);
    bounds[3] = std::max(bounds[3], shape->bounds[3]);

    for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
      for (int i = 0; i < path->npts - 1; i += 3) {
        float *p = &path->pts[i * 2];
        PolygonF polygon = Bezier::fromPoints({p[0], p[1]}, {p[2], p[3]},
                                              {p[4], p[5]}, {p[6], p[7]})
                               .toPolygon();
        for (auto &line : polygon) {
          // Draw Line
        }
      }
    } // end for
  }
  // Delete
  nsvgDelete(image);

  printf("origin bounds: %.2f %.2f %.2f %.2f\n\n", bounds[0], bounds[1],
         bounds[2], bounds[3]);

  while (1) {
    PerformanceTimer cost;
    cost.start();
    SVGParser parser;
    parser.open(svg, "px", 96);
    parser.parser();                    // 获取缩放系数
    parser.repaser();                   // 获取缩放后的视图大小
    parser.render(0, 0, 0, NULL, 0, 0); // 渲染
    parser.close();
    cost.stop();
    std::println("cost:{}", cost.elapsedMilliseconds());
    return 0;
  }

  return 0;
}
