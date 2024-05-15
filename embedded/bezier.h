
#ifndef BEZIER_P_H
#define BEZIER_P_H

#include <vector>

struct PointF {
  float x, y;
};

struct LineF {
  PointF start, end;
};

using PolygonF = std::vector<LineF>;

struct CubicBezierCurve {
  PointF start;
  PointF control1;
  PointF control2;
  PointF end;
};

// ref https://en.wikipedia.org/wiki/De_Casteljau%27s_algorithm
class Bezier {
public:
  static Bezier fromPoints(const PointF &p1, const PointF &p2, const PointF &p3,
                           const PointF &p4) {
    return {p1,p2,p3,p4};
  }

  PolygonF toPolygon(float bezier_flattening_threshold = 0.5);

  CubicBezierCurve mCurve;
};

#endif // BEZIER_P_H
