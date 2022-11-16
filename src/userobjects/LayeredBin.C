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

#include "LayeredBin.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("CardinalApp", LayeredBin);

InputParameters
LayeredBin::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  MooseEnum directions("x y z");

  params.addRequiredParam<MooseEnum>(
      "direction", directions, "The direction of the layers (x, y, or z)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_layers",
      "num_layers > 0",
      "The number of layers between the bounding box of the domain");
  params.addClassDescription("Creates a unique spatial bin for layers in a specified direction");
  return params;
}

LayeredBin::LayeredBin(const InputParameters & parameters)
  : SpatialBinUserObject(parameters),
    _direction(parameters.get<MooseEnum>("direction")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _layered_subproblem(parameters.getCheckedPointerParam<SubProblem *>("_subproblem"))
{
  BoundingBox bounding_box = MeshTools::create_bounding_box(_layered_subproblem->mesh());
  _direction_min = bounding_box.min()(_direction);
  _direction_max = bounding_box.max()(_direction);

  _directions = {_direction};

  _layer_pts.resize(_num_layers + 1);
  _layer_pts[0] = _direction_min;
  Real dx = (_direction_max - _direction_min) / _num_layers;
  for (unsigned int i = 1; i < _num_layers + 1; ++i)
    _layer_pts[i] = _layer_pts[i - 1] + dx;

  _bin_centers.resize(_num_layers);
  for (unsigned int i = 0; i < _num_layers; ++i)
  {
    _bin_centers[i] = Point(0.0, 0.0, 0.0);
    _bin_centers[i](_direction) = 0.5 * (_layer_pts[i + 1] + _layer_pts[i]);
  }
}

unsigned int
LayeredBin::bin(const Point & p) const
{
  Real direction_x = p(_direction);
  return binFromBounds(direction_x, _layer_pts);
}

unsigned int
LayeredBin::num_bins() const
{
  return _num_layers;
}
