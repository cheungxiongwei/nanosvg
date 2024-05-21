#if __has_include("nanosparser.hpp")
#include "nanosparser.hpp"
#endif
#include "PerformanceTimer.hpp"
#include "nanosvgrast.h"
#include <algorithm>
#include <cfloat>

#include <stdio.h>

#include "bezier.h"

#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>

struct Transform {
  float dx, dy, sx, sy, scale;
};

int main() {
  cv::utils::logging::setLogLevel(
      cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);

  constexpr auto svg = R"(C:\Users\cheungxiongwei\Documents\test(1).svg)";
  //constexpr auto svg = R"(C:\Users\cheungxiongwei\Documents\tig.svg)";

  // Load SVG
  NSVGimage *image;
  image = nsvgParseFromFile(svg, "px", 96);
  printf("size: %f x %f\n", image->width, image->height);

  cv::Mat mat = cv::Mat::zeros(cv::Size(640, 480), CV_8UC4);

  Transform T;
  T.sx = mat.size().width / image->width;
  T.sy = mat.size().height / image->height;
  T.scale = T.sx < T.sy ? T.sx : T.sy;
  T.dx = (mat.size().width - image->width * T.scale) / 2.f;
  T.dy = (mat.size().height - image->height * T.scale) / 2.F;

  // Use...
  float bounds[4] = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
  for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
    bounds[0] = std::min(bounds[0], shape->bounds[0]);
    bounds[1] = std::min(bounds[1], shape->bounds[1]);
    bounds[2] = std::max(bounds[2], shape->bounds[2]);
    bounds[3] = std::max(bounds[3], shape->bounds[3]);

    for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
      for (int i = 0; i < path->npts - 1; i += 3) {
        float *p = &path->pts[i * 2];
        CubicBezierCurve curve = Bezier::fromPoints({p[0], p[1]}, {p[2], p[3]},
                                                    {p[4], p[5]}, {p[6], p[7]});
        PolygonF polygon = Bezier::toPolygon(curve);
        for (auto i = 1; i < polygon.size(); i++) {
          // Draw Line
          // polygon[i-1],polygon[i]
          auto p1 = polygon[i - 1];
          auto p2 = polygon[i];
          cv::line(
              mat,
              cv::Point{int(p1.x * T.scale + T.dx), int(p1.y * T.scale + T.dy)},
              cv::Point{int(p2.x * T.scale + T.dx), int(p2.y * T.scale + T.dy)},
              cv::Scalar(0, 255, 0), 1);
        }
      }
    } // end for
  }
  // Delete
  nsvgDelete(image);

  printf("origin bounds: %.2f %.2f %.2f %.2f\n\n", bounds[0], bounds[1],
         bounds[2], bounds[3]);

  cv::imshow("mat", mat);
  cv::waitKeyEx();

  while (1) {
#if 1
    PerformanceTimer cost;
    cost.start();

    SVGParser parser;
    parser.open(svg);
    parser.parser();                    // 解析
    parser.render(0, 0, 0, NULL, 0, 0); // 渲染
    parser.close();

    cost.stop();
    std::println("cost:{}", cost.elapsedMilliseconds());
#endif
    system("pause");
    return 0;
  }

  return 0;
}
