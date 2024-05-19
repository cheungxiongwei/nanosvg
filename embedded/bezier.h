
#ifndef BEZIER_H
#define BEZIER_H

#include <vector>

struct PointF {
  float x, y;
};

using PolygonF = std::vector<PointF>;

struct CubicBezierCurve {
  PointF start;
  PointF control1;
  PointF control2;
  PointF end;
};

class Bezier {
public:
  static CubicBezierCurve fromPoints(const PointF &p1, const PointF &p2,
                                     const PointF &p3, const PointF &p4) {
    return {p1, p2, p3, p4};
  }

  static PolygonF toPolygon(const CubicBezierCurve &curve,
                            float bezier_flattening_threshold = 0.5);
};

// embedded 
struct EmbeddedPolygonF {
  PointF data[64];
  uint16_t count;
};

void flattenCubicBezier(const CubicBezierCurve &curve,
                        EmbeddedPolygonF *polygon,
                        float bezier_flattening_threshold = 0.5);

#endif // BEZIER_H
