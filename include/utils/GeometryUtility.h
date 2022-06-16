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

#pragma once

#include "Moose.h"
#include "libmesh/point.h"

namespace geom_utility {
  /**
   * Get the indices of the plane perpendicular to the specified axis.
   * For example, if the axis is the y-axis (1), then this will return
   * (0, 2), indicating that the coordinates of a general 3-D point once
   * projected onto the x-z plane can be obtained with the 0 and 2 indices.
   * @param[in] axis axis perpendicular to the projection plane
   * @return indices of coordinates on plane
   */
  std::pair<unsigned int, unsigned int> projectedIndices(const unsigned int & axis);

  /**
   * Given two coordinates, construct a point in the 2-D plane perpendicular to the
   * specified axis.
   * @param[in] x0 first coordinate
   * @param[in] x1 second coordinate
   * @param[in] axis axis perpendicular to the projection plane
   * @return point
   */
  Point projectPoint(const Real & x0, const Real & x1, const unsigned int & axis);

  /**
   * Get the unit normal vector between two points (which are first projected onto
   * the plane perpendicular to the 'axis'), such that the cross product of
   * the unit normal with the line from pt1 to pt2 has a positive 'axis' component.
   * @param[in] pt1 first point for line
   * @param[in] pt2 second point for line
   * @return unit normal
   */
  Point projectedUnitNormal(Point pt1, Point pt2, const unsigned int & axis);

  /**
   * Compute the distance from a 3-D line, provided in terms of two points on the line
   * @param[in] pt point of interest
   * @param[in] line0 first point on line
   * @param[in] line1 second point on line
   * @return distance from line
   */
  Real distanceFromLine(const Point & pt, const Point & line0, const Point & line1);

  /**
   * Compute the distance from a 3-D line, provided in terms of two points on the line.
   * Both the input point and the points on the line are projected into the 2-d plane
   * perpendicular to the specified axis.
   * @param[in] pt point of interest
   * @param[in] line0 first point on line
   * @param[in] line1 second point on line
   * @param[in] axis axis index (0 = x, 1 = y, 2 = z) perpendicular to the projection plane
   * @return distance from line
   */
  Real projectedDistanceFromLine(Point pt, Point line0, Point line1, const unsigned int & axis);

  /**
   * If positive, point is on the positive side of the half space (and vice versa). Because
   * a 3-D line does not have a negative or positive "side," you must provide the 'axis'
   * perpendicular to the plane into which the point and line are first projected.
   * @param[in] pt1 point of interest
   * @param[in] pt2 one end point of line
   * @param[in] pt3 other end point of line
   * @param[in] axis axis perpendicular to plane onto which point and line are first projected
   * @return half space of line
   */
  const Real projectedLineHalfSpace(Point pt1, Point pt2, Point pt3, const unsigned int & axis);

  /**
   * Whether a point is in a polygon given by corner points
   * @param[in] point point of interest
   * @param[in] corners corner points of polygon
   * @return whether point is inside the polygon
   */
  const bool pointInPolygon(const Point & point, const std::vector<Point> & corners);

  /**
   * Whether a point is on the edge of a polygon given its corner points
   * @param[in] point point of interest
   * @param[in] corners corner points of polygon
   * @return whether point is on edge of polygon
   */
  const bool pointOnEdge(const Point & point, const std::vector<Point> & corners);
}; // end of namespace geom_utility
