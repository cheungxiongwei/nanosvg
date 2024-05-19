#include "bezier.h"

// De Casteljau's algorithm for cubic Bezier subdivision
static void FlattenCubicBezierRecursive(const CubicBezierCurve &curve,
                                        float bezier_flattening_threshold,
                                        std::vector<PointF> &result) {
  CubicBezierCurve beziers[10];
  int levels[10];
  beziers[0] = curve;
  levels[0] = 9;
  int top = 0;

  while (top >= 0) {
    CubicBezierCurve *b = &beziers[top];

    float y4y1 = b->end.y - b->start.y;
    float x4x1 = b->end.x - b->start.x;
    float l = std::abs(x4x1) + std::abs(y4y1);
    float d;
    if (l > 1.) {
      d = std::abs((x4x1) * (b->start.y - b->control1.y) -
                   (y4y1) * (b->start.x - b->control1.x)) +
          std::abs((x4x1) * (b->start.y - b->control2.y) -
                   (y4y1) * (b->start.x - b->control2.x));
    } else {
      d = std::abs(b->start.x - b->control1.x) +
          std::abs(b->start.y - b->control1.y) +
          std::abs(b->start.x - b->control2.x) +
          std::abs(b->start.y - b->control2.y);
      l = 1.;
    }
    if (d < bezier_flattening_threshold * l || levels[top] == 0) {
      result.emplace_back(b->end);
      --top;
    } else {

      auto fn_split = [](const CubicBezierCurve *curve)
          -> std::pair<CubicBezierCurve, CubicBezierCurve> {
        const auto mid = [](PointF lhs, PointF rhs) -> PointF {
          return PointF{((lhs.x + rhs.x) * 0.5f), ((lhs.y + rhs.y) * 0.5f)};
        };

        const PointF mid_12 = mid(curve->start, curve->control1);
        const PointF mid_23 = mid(curve->control1, curve->control2);
        const PointF mid_34 = mid(curve->control2, curve->end);
        const PointF mid_12_23 = mid(mid_12, mid_23);
        const PointF mid_23_34 = mid(mid_23, mid_34);
        const PointF mid_12_23__23_34 = mid(mid_12_23, mid_23_34);

        return {
            Bezier::fromPoints(curve->start, mid_12, mid_12_23,
                               mid_12_23__23_34),
            Bezier::fromPoints(mid_12_23__23_34, mid_23_34, mid_34, curve->end),
        };
      };

      std::tie(b[1], b[0]) = fn_split(b);
      levels[top + 1] = --levels[top];
      ++top;
    }
  }
}

// De Casteljau's algorithm for cubic Bezier subdivision
static void
EmbeddedFlattenCubicBezierRecursive(const CubicBezierCurve &curve,
                                    float bezier_flattening_threshold,
                                    EmbeddedPolygonF *result) {
  CubicBezierCurve beziers[10];
  int levels[10];
  beziers[0] = curve;
  levels[0] = 9;
  int top = 0;

  while (top >= 0) {
    CubicBezierCurve *b = &beziers[top];

    float y4y1 = b->end.y - b->start.y;
    float x4x1 = b->end.x - b->start.x;
    float l = std::abs(x4x1) + std::abs(y4y1);
    float d;
    if (l > 1.) {
      d = std::abs((x4x1) * (b->start.y - b->control1.y) -
                   (y4y1) * (b->start.x - b->control1.x)) +
          std::abs((x4x1) * (b->start.y - b->control2.y) -
                   (y4y1) * (b->start.x - b->control2.x));
    } else {
      d = std::abs(b->start.x - b->control1.x) +
          std::abs(b->start.y - b->control1.y) +
          std::abs(b->start.x - b->control2.x) +
          std::abs(b->start.y - b->control2.y);
      l = 1.;
    }
    if (d < bezier_flattening_threshold * l || levels[top] == 0) {
      if (result->count > 63)
        result->count = 63;

      result->data[result->count++] = PointF(b->end.x, b->end.y);
      --top;
    } else {

      auto fn_split = [](const CubicBezierCurve *curve)
          -> std::pair<CubicBezierCurve, CubicBezierCurve> {
        const auto mid = [](PointF lhs, PointF rhs) -> PointF {
          return PointF{((lhs.x + rhs.x) * 0.5f), ((lhs.y + rhs.y) * 0.5f)};
        };

        const PointF mid_12 = mid(curve->start, curve->control1);
        const PointF mid_23 = mid(curve->control1, curve->control2);
        const PointF mid_34 = mid(curve->control2, curve->end);
        const PointF mid_12_23 = mid(mid_12, mid_23);
        const PointF mid_23_34 = mid(mid_23, mid_34);
        const PointF mid_12_23__23_34 = mid(mid_12_23, mid_23_34);

        return {
            Bezier::fromPoints(curve->start, mid_12, mid_12_23,
                               mid_12_23__23_34),
            Bezier::fromPoints(mid_12_23__23_34, mid_23_34, mid_34, curve->end),
        };
      };

      std::tie(b[1], b[0]) = fn_split(b);
      levels[top + 1] = --levels[top];
      ++top;
    }
  }
}

PolygonF Bezier::toPolygon(const CubicBezierCurve &curve,
                           float bezier_flattening_threshold) {
  PolygonF flattenedCurve;
  flattenedCurve.emplace_back(curve.start);
  FlattenCubicBezierRecursive(curve, bezier_flattening_threshold,
                              flattenedCurve);
  return flattenedCurve;
}

void cpp::flattenCubicBezier(const CubicBezierCurve &curve,
                             EmbeddedPolygonF *polygon,
                             float bezier_flattening_threshold) {
  if (polygon) {
    polygon->count = 0;
    polygon->data[polygon->count++] = curve.start;
    EmbeddedFlattenCubicBezierRecursive(curve, bezier_flattening_threshold,
                                        polygon);
  }
}
