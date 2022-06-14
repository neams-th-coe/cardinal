/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#include "GeometryUtility.h"

namespace geom_utility {

const Real lineHalfSpace(const Point & pt1, const Point & pt2, const Point & pt3)
{
  return (pt1(0) - pt3(0)) * (pt2(1) - pt3(1)) - (pt2(0) - pt3(0)) * (pt1(1) - pt3(1));
}

const bool pointInPolygon(const Point & point, const std::vector<Point> & corners)
{
  auto n_pts = corners.size();

  std::vector<bool> negative_half_space;
  std::vector<bool> positive_half_space;
  for (unsigned int i = 0; i < corners.size(); ++i)
  {
    int next = (i == n_pts - 1) ? 0 : i + 1;
    auto half = lineHalfSpace(point, corners[i], corners[next]);
    negative_half_space.push_back(half < 0);
    positive_half_space.push_back(half > 0);
  }

  bool negative = std::find(negative_half_space.begin(), negative_half_space.end(), true) !=
                  negative_half_space.end();
  bool positive = std::find(positive_half_space.begin(), positive_half_space.end(), true) !=
                  positive_half_space.end();

  bool in_polygon = !(negative && positive);
  if (in_polygon) return true;

  for (unsigned int i = 0; i < corners.size(); ++i)
    if (pointOnEdge(point, corners))
      return true;

  return false;
}

const bool pointOnEdge(const Point & point, const std::vector<Point> & corners)
{
  auto n_pts = corners.size();

  Real tol = 1e-8;
  for (unsigned int i = 0; i < corners.size(); ++i)
  {
    int next = (i == n_pts - 1) ? 0 : i + 1;
    const auto & pt1 = corners[i];
    const auto & pt2 = corners[next];
    bool close_to_line = distanceFromLine(point, pt1, pt2) < tol;

    // we can stop early if we know we're not close to the line
    if (!close_to_line)
      continue;

    // check that the point is "between" the two points; TODO: first pass
    // we can just compare x and y coordinates
    bool between_points = (point(0) >= std::min(pt1(0), pt2(0))) &&
                          (point(0) <= std::max(pt1(0), pt2(0))) &&
                          (point(1) >= std::min(pt1(1), pt2(1))) &&
                          (point(1) <= std::max(pt1(1), pt2(1)));

    // point needs to be close to the line AND "between" the two points
    if (close_to_line && between_points)
      return true;
  }

  return false;
}

Point unitNormal(const Point & pt1, const Point & pt2)
{
  Real dx = pt2(0) - pt1(0);
  Real dy = pt2(1) - pt1(1);
  Point normal = {dy, -dx, 0.0};
  Point gap_line = pt2 - pt1;

  auto cross_product = gap_line.cross(normal);

  if (cross_product(2) > 0)
    return normal.unit();
  else
  {
    Point corrected_normal = {-dy, dx, 0.0};
    return corrected_normal.unit();
  }
}

std::vector<Real> getLineCoefficients(const Point & line0, const Point & line1)
{
  std::vector<Real> l;
  l.resize(3);

  // vertical line needs special treatment
  if (line1(0) == line0(0))
  {
    l[0] = 1.0;
    l[1] = 0.0;
    l[2] = -line1(0);
  }
  else
  {
    l[0] = (line1(1) - line0(1)) / (line1(0) - line0(0));
    l[1] = -1.0;
    l[2] = line1(1) - l[0] * line1(0);
  }

  return l;
}

Real distanceFromLine(const Point & pt, const Point & line0, const Point & line1)
{
  auto l = getLineCoefficients(line0, line1);
  return std::abs(l[0] * pt(0) + l[1] * pt(1) + l[2]) / std::sqrt(l[0] * l[0] + l[1] * l[1]);
}

}; // end namespace geom_utility
