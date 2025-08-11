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

#include "HexagonalSubchannelGapBin.h"

registerMooseObject("CardinalApp", HexagonalSubchannelGapBin);

InputParameters
HexagonalSubchannelGapBin::validParams()
{
  InputParameters params = PlaneSpatialBinUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "bundle_pitch", "bundle_pitch > 0", "Bundle pitch, or flat-to-flat distance across bundle");
  params.addRequiredRangeCheckedParam<Real>(
      "pin_pitch", "pin_pitch > 0", "Pin pitch, or distance between pin centers");
  params.addRequiredRangeCheckedParam<Real>(
      "pin_diameter", "pin_diameter > 0", "Pin outer diameter");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_rings", "n_rings >= 1", "Number of pin rings, including the centermost pin as a 'ring'");

  MooseEnum directions("x y z", "z");
  params.addParam<MooseEnum>(
      "axis", directions, "vertical axis of the reactor (x, y, or z) along which pins are aligned");
  params.addClassDescription(
      "Creates a unique spatial bin for each subchannel in a hexagonal lattice");
  return params;
}

HexagonalSubchannelGapBin::HexagonalSubchannelGapBin(const InputParameters & parameters)
  : PlaneSpatialBinUserObject(parameters),
    _bundle_pitch(getParam<Real>("bundle_pitch")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_rings(getParam<unsigned int>("n_rings")),
    _axis(parameters.get<MooseEnum>("axis"))
{
  _hex_lattice.reset(new HexagonalLatticeUtils(_bundle_pitch,
                                               _pin_pitch,
                                               _pin_diameter,
                                               0.0 /* wire diameter, unused */,
                                               1.0 /* wire pitch, unused */,
                                               _n_rings,
                                               _axis,
                                               0. /*rotation_around_axis*/));

  if (_axis == 0) // x vertical axis
    _directions = {1, 2};
  else if (_axis == 1) // y vertical axis
    _directions = {0, 2};
  else // z vertical axis
    _directions = {0, 1};

  // the bin centers are the gap centers
  const auto & gap_centers = _hex_lattice->gapCenters();
  for (const auto & gap : gap_centers)
    _bin_centers.push_back(gap);
}

unsigned int
HexagonalSubchannelGapBin::bin(const Point & p) const
{
  return _hex_lattice->gapIndex(p);
}

unsigned int
HexagonalSubchannelGapBin::num_bins() const
{
  return _hex_lattice->nGaps();
}

Real
HexagonalSubchannelGapBin::distanceFromGap(const Point & point,
                                           const unsigned int & gap_index) const
{
  return _hex_lattice->distanceFromGap(point, gap_index);
}

unsigned int
HexagonalSubchannelGapBin::gapIndex(const Point & point) const
{
  return _hex_lattice->gapIndex(point);
}

void
HexagonalSubchannelGapBin::gapIndexAndDistance(const Point & point,
                                               unsigned int & index,
                                               Real & distance) const
{
  _hex_lattice->gapIndexAndDistance(point, index, distance);
}
