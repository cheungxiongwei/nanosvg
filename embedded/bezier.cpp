#include "bezier.h"

// Calculate distance between two points
static float distance(const PointF &p1, const PointF &p2) {
  float dx = p2.x - p1.x;
  float dy = p2.y - p1.y;
  return sqrt(dx * dx + dy * dy);
}

// Subdivide a cubic Bezier curve into two halves
static void subdivideCubicBezier(const CubicBezierCurve &curve,
                                 CubicBezierCurve &left,
                                 CubicBezierCurve &right) {
  PointF mid1, mid2, mid3, mid4, mid5, mid6;
  mid1.x = (curve.start.x + curve.control1.x) / 2;
  mid1.y = (curve.start.y + curve.control1.y) / 2;
  mid2.x = (curve.control1.x + curve.control2.x) / 2;
  mid2.y = (curve.control1.y + curve.control2.y) / 2;
  mid3.x = (curve.control2.x + curve.end.x) / 2;
  mid3.y = (curve.control2.y + curve.end.y) / 2;

  mid4.x = (mid1.x + mid2.x) / 2;
  mid4.y = (mid1.y + mid2.y) / 2;

  mid5.x = (mid2.x + mid3.x) / 2;
  mid5.y = (mid2.y + mid3.y) / 2;

  mid6.x = (mid4.x + mid5.x) / 2;
  mid6.y = (mid4.y + mid5.y) / 2;

  left.start = curve.start;
  left.control1 = mid1;
  left.control2 = mid4;
  left.end = mid6;

  right.start = mid6;
  right.control1 = mid5;
  right.control2 = mid3;
  right.end = curve.end;
}

static void flattenCubicBezierRecursive(const CubicBezierCurve &curve,
                                 float tolerance,
                                 std::vector<PointF> &result) {
  // Calculate the length of the curve
  float length = distance(curve.start, curve.end);

  // Calculate the distance between the control points and the midpoint
  float control1_mid = distance(curve.start, curve.control1) +
                        distance(curve.control1, curve.end);
  float control2_mid = distance(curve.start, curve.control2) +
                        distance(curve.control2, curve.end);

  // Check if the curve needs further subdivision
  if (control1_mid + control2_mid - length > tolerance) {
    // Subdivide the curve
    CubicBezierCurve left, right;
    subdivideCubicBezier(curve, left, right);
    // Recursively flatten each half
    flattenCubicBezierRecursive(left, tolerance, result);
    flattenCubicBezierRecursive(right, tolerance, result);
  } else {
    // If the curve doesn't need further subdivision, add the endpoint to the
    // result
    result.push_back(curve.end);
  }
}

// Flatten a cubic Bezier curve into line segments
static std::vector<PointF> flattenCubicBezier(const CubicBezierCurve &curve,
                                              float tolerance) {
  std::vector<PointF> flattenedCurve;
  flattenCubicBezierRecursive(curve, tolerance, flattenedCurve);
  return flattenedCurve;
}

PolygonF Bezier::toPolygon(float bezier_flattening_threshold) {
  PolygonF polygon;
  auto points = flattenCubicBezier(mCurve, bezier_flattening_threshold);
  for (size_t i = 1; i < points.size(); i++) {
    polygon.emplace_back(LineF{points[i - 1], points[i]});
  }
  return polygon;
}
