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

#ifdef ENABLE_NEK_COUPLING

#include "NekPlaneSpatialBinUserObject.h"

InputParameters
NekPlaneSpatialBinUserObject::validParams()
{
  InputParameters params = NekSpatialBinUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>("gap_thickness", "gap_thickness > 0.0",
    "thickness of gap region for which to accept contributions to the side integral over "
    "the gap, expressed in the same units as the mesh.");
  return params;
}

NekPlaneSpatialBinUserObject::NekPlaneSpatialBinUserObject(const InputParameters & parameters)
  : NekSpatialBinUserObject(parameters),
    _gap_thickness(getParam<Real>("gap_thickness"))
{
  // we need to enforce that there is only one side distribution, because side
  // distributions defined in orthogonal directions don't ever overlap with one another
  unsigned int num_side_distributions = 0;

  for (unsigned int i = 0; i < _bins.size(); ++i)
  {
    auto & uo = _bins[i];
    const PlaneSpatialBinUserObject * side = dynamic_cast<const PlaneSpatialBinUserObject *>(uo);
    if (side)
    {
      ++num_side_distributions;
      _side_bin = side;
      _side_index = i;
    }
  }

  if (num_side_distributions != 1)
    mooseError("This user object requires exactly one bin distribution "
      "to be a side distribution; you have specified: " + Moose::stringify(num_side_distributions) +
      "\noptions: HexagonalSubchannelGapBin");

  if (_field == field::velocity_component && _velocity_component == component::normal)
  {
    if (!_fixed_mesh)
      mooseError("The gap unit normals assume the NekRS domain is not moving; with a moving "
        "mesh, the 'velocity_component = normal' setting is unavailable unless internal methods "
        "are updated to recompute normals following a change in geometry.");

    _velocity_bin_directions = _side_bin->gapUnitNormals();
  }
}

Real
NekPlaneSpatialBinUserObject::distanceFromGap(const Point & point, const unsigned int & gap_index) const
{
  return _side_bin->distanceFromGap(point, gap_index);
}

unsigned int
NekPlaneSpatialBinUserObject::gapIndex(const Point & point) const
{
  return _side_bin->gapIndex(point);
}

void
NekPlaneSpatialBinUserObject::gapIndexAndDistance(const Point & point, unsigned int & index,  Real & distance) const
{
  _side_bin->gapIndexAndDistance(point, index, distance);
}

#endif
