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

#include "RadialBin.h"

registerMooseObject("CardinalApp", RadialBin);

InputParameters
RadialBin::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  MooseEnum directions("x y z");

  params.addRequiredParam<MooseEnum>(
      "vertical_axis",
      directions,
      "The vertical axis about which to compute the radial coordinate (x, y, or z)");
  params.addRangeCheckedParam<Real>(
      "rmin",
      0.0,
      "rmin >= 0.0",
      "Inner radius. Setting 'rmin = 0' corresponds to a cross-section of a circle");
  params.addRequiredRangeCheckedParam<Real>("rmax", "rmax > 0.0", "Outer radius");

  params.addRequiredRangeCheckedParam<unsigned int>(
      "nr", "nr > 0", "The number of layers in the radial direction");
  params.addRangeCheckedParam<Real>("growth_r",
                                    1.0,
                                    "growth_r > 0.0",
                                    "The ratio of radial sizes of successive rings of elements");

  params.addClassDescription("Creates spatial bins for layers in the radial direction");
  return params;
}

RadialBin::RadialBin(const InputParameters & parameters)
  : SpatialBinUserObject(parameters),
    _vertical_axis(parameters.get<MooseEnum>("vertical_axis")),
    _rmin(getParam<Real>("rmin")),
    _rmax(getParam<Real>("rmax")),
    _nr(getParam<unsigned int>("nr")),
    _growth_r(getParam<Real>("growth_r"))
{
  if (_rmax <= _rmin)
    mooseError("Maximum radial coordinate 'rmax' must be greater than the minimum radial "
               "coordinate 'rmin'!");

  Real first_width = _growth_r == 1.0 ? (_rmax - _rmin) / _nr
                                      : (_rmax - _rmin) * (1.0 - std::abs(_growth_r)) /
                                            (1.0 - std::pow(std::abs(_growth_r), _nr));

  _radial_pts.resize(_nr + 1);
  _radial_pts[0] = _rmin;
  _radial_pts[1] = _radial_pts[0] + first_width;

  for (unsigned int i = 2; i < _nr + 1; ++i)
  {
    Real dr = _growth_r * (_radial_pts[i - 1] - _radial_pts[i - 2]);
    _radial_pts[i] = _radial_pts[i - 1] + dr;
  }

  if (_vertical_axis == 0) // x vertical axis
    _directions = {1, 2};
  else if (_vertical_axis == 1) // y vertical axis
    _directions = {0, 2};
  else // z vertical axis
    _directions = {0, 1};

  _bin_centers.resize(_nr);
  for (unsigned int i = 0; i < _nr; ++i)
  {
    _bin_centers[i] = Point(0.0, 0.0, 0.0);
    _bin_centers[i](_directions[0]) = 0.5 * (_radial_pts[i + 1] + _radial_pts[i]);
  }
}

unsigned int
RadialBin::bin(const Point & p) const
{
  Real r = std::sqrt(p.norm_sq() - p(_vertical_axis) * p(_vertical_axis));
  return binFromBounds(r, _radial_pts);
}

unsigned int
RadialBin::num_bins() const
{
  return _nr;
}
