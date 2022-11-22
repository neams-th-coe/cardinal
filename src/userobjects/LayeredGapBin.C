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

#include "LayeredGapBin.h"

registerMooseObject("CardinalApp", LayeredGapBin);

InputParameters
LayeredGapBin::validParams()
{
  InputParameters params = PlaneSpatialBinUserObject::validParams();
  MooseEnum directions("x y z");

  params.addRequiredParam<MooseEnum>(
      "direction", directions, "The direction of the layers (x, y, or z)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_layers",
      "num_layers > 0",
      "The number of layers between the bounding box of the domain; this will "
      "create num_layers + 1 side bins");
  params.addClassDescription("Creates a unique spatial bin for layers in a specified direction");
  return params;
}

LayeredGapBin::LayeredGapBin(const InputParameters & parameters)
  : PlaneSpatialBinUserObject(parameters),
    _direction(parameters.get<MooseEnum>("direction")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _layered_subproblem(parameters.getCheckedPointerParam<SubProblem *>("_subproblem")),
    _num_faces(_num_layers + 1)
{
  BoundingBox bounding_box = MeshTools::create_bounding_box(_layered_subproblem->mesh());
  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);

  _directions = {_direction};

  _layer_pts.resize(_num_faces);
  _layer_pts[0] = _direction_min;
  Real dx = (_direction_max - _direction_min) / _num_layers;
  for (unsigned int i = 1; i < _num_faces; ++i)
    _layer_pts[i] = _layer_pts[i - 1] + dx;

  _bin_centers.resize(_num_faces);
  _unit_normals.resize(_num_faces);
  for (unsigned int i = 0; i < _num_faces; ++i)
  {
    _bin_centers[i] = Point(0.0, 0.0, 0.0);
    _bin_centers[i](_direction) = _layer_pts[i];

    _unit_normals[i] = Point(0.0, 0.0, 0.0);
    _unit_normals[i](_direction) = 1.0;
  }

  _effective_layer_pts.resize(_num_layers + 2);
  _effective_layer_pts[0] = _direction_min - dx;
  _effective_layer_pts[_num_layers + 1] = _direction_max + dx;
  for (unsigned int i = 1; i < _num_layers + 1; ++i)
    _effective_layer_pts[i] = 0.5 * (_layer_pts[i] + _layer_pts[i - 1]);
}

unsigned int
LayeredGapBin::bin(const Point & p) const
{
  Real direction_x = p(_direction);
  return binFromBounds(direction_x, _effective_layer_pts);
}

unsigned int
LayeredGapBin::num_bins() const
{
  return _num_faces;
}

unsigned int
LayeredGapBin::gapIndex(const Point & p) const
{
  return bin(p);
}

Real
LayeredGapBin::distanceFromGap(const Point & point, const unsigned int & gap_index) const
{
  const auto & layer_pt = _layer_pts[gap_index];
  return std::abs(point(_direction) - layer_pt);
}

void
LayeredGapBin::gapIndexAndDistance(const Point & point, unsigned int & index, Real & distance) const
{
  index = bin(point);
  distance = distanceFromGap(point, index);
}

Real
LayeredGapBin::adjustBinValue(const unsigned int & i) const
{
  // This bin object gets _direction_min and _direction_max from a bounding box over the
  // mesh. This means that the first and last planes are on the boundary of the domain,
  // so the integrating volume is actually only half of _gap_thickness in
  // NekBinnedPlaneIntegral/NekBinnedPlaneAverage. Therefore, to get correct area integrals,
  // we divide each integral by _gap_thickness (in NekSideBinnedIntegral) except for the
  // first and last bins, which we only divide by _gap_thickness / 2.0

  if (i == 0 || i == (num_bins() - 1))
    return 2.0;

  return 1.0;
}
