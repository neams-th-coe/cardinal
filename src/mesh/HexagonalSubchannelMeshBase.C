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

#include "HexagonalSubchannelMeshBase.h"

const Real HexagonalSubchannelMeshBase::COS30 = std::sqrt(3.0) / 2.0;
const Real HexagonalSubchannelMeshBase::SIN30 = 0.5;
const unsigned int HexagonalSubchannelMeshBase::NODES_PER_PRISM = 6;
const unsigned int HexagonalSubchannelMeshBase::NUM_SIDES = 6;

InputParameters
HexagonalSubchannelMeshBase::validParams()
{
  InputParameters params = MooseMesh::validParams();
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
  return params;
}

HexagonalSubchannelMeshBase::HexagonalSubchannelMeshBase(const InputParameters & parameters)
  : MooseMesh(parameters),
    _bundle_pitch(getParam<Real>("bundle_pitch")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_rings(getParam<unsigned int>("n_rings")),
    _axis(parameters.get<MooseEnum>("axis")),
    _hex_lattice(HexagonalLatticeUtils(
        _bundle_pitch,
        _pin_pitch,
        _pin_diameter,
        0.0 /* wire diameter not needed for subchannel mesh, use dummy value */,
        1.0 /* wire pitch not needed for subchannel mesh, use dummy value */,
        _n_rings,
        _axis,
        0. /*rotation_around_axis*/)),
    _pin_centers(_hex_lattice.pinCenters())
{
}

const Point
HexagonalSubchannelMeshBase::rotatePoint(const Point & p, const Real & theta) const
{
  Real x = p(0);
  Real y = p(1);
  Point rotation(
      x * std::cos(theta) - y * std::sin(theta), x * std::sin(theta) + y * std::cos(theta), 0.0);
  return rotation;
}
