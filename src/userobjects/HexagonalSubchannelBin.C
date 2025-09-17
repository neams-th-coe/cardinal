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

#include "HexagonalSubchannelBin.h"

registerMooseObject("CardinalApp", HexagonalSubchannelBin);

InputParameters
HexagonalSubchannelBin::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>(
      "bundle_pitch", "bundle_pitch > 0", "Bundle pitch, or flat-to-flat distance across bundle");
  params.addRequiredRangeCheckedParam<Real>(
      "pin_pitch", "pin_pitch > 0", "Pin pitch, or distance between pin centers");
  params.addRequiredRangeCheckedParam<Real>(
      "pin_diameter", "pin_diameter > 0", "Pin outer diameter");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "n_rings", "n_rings >= 1", "Number of pin rings, including the centermost pin as a 'ring'");
  params.addParam<bool>(
      "pin_centered_bins",
      false,
      "Whether the bins should be channel-centered (false) or pin-centered (true)");

  MooseEnum directions("x y z", "z");
  params.addParam<MooseEnum>(
      "axis", directions, "vertical axis of the reactor (x, y, or z) along which pins are aligned");
  params.addClassDescription(
      "Creates a unique spatial bin for each subchannel in a hexagonal lattice");
  return params;
}

HexagonalSubchannelBin::HexagonalSubchannelBin(const InputParameters & parameters)
  : SpatialBinUserObject(parameters),
    _bundle_pitch(getParam<Real>("bundle_pitch")),
    _pin_pitch(getParam<Real>("pin_pitch")),
    _pin_diameter(getParam<Real>("pin_diameter")),
    _n_rings(getParam<unsigned int>("n_rings")),
    _axis(parameters.get<MooseEnum>("axis")),
    _pin_centered_bins(getParam<bool>("pin_centered_bins"))
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

  if (_pin_centered_bins)
  {
    // the bin centers are the pin centroids
    for (const auto & p : _hex_lattice->pinCenters())
      _bin_centers.push_back(p);

    // except for the last bin, which is just the enclosing region; we
    // just pick one point in this region
    Real x;
    if (_n_rings % 2 == 0)
      x = 0.0;
    else
      x = 0.5 * _hex_lattice->pinPitch();

    Point leftover(x, _bundle_pitch / 2.0 - _hex_lattice->pinBundleSpacing() / 2.0, 0.0);
    _bin_centers.push_back(leftover);
  }
  else
  {
    // the bin centers are the channel centroids
    for (unsigned int i = 0; i < _hex_lattice->nInteriorChannels(); ++i)
    {
      auto corners = _hex_lattice->interiorChannelCornerCoordinates(i);
      _bin_centers.push_back(_hex_lattice->channelCentroid(corners));
    }

    for (unsigned int i = 0; i < _hex_lattice->nEdgeChannels(); ++i)
    {
      auto corners = _hex_lattice->edgeChannelCornerCoordinates(i);
      _bin_centers.push_back(_hex_lattice->channelCentroid(corners));
    }

    for (unsigned int i = 0; i < _hex_lattice->nCornerChannels(); ++i)
    {
      auto corners = _hex_lattice->cornerChannelCornerCoordinates(i);
      _bin_centers.push_back(_hex_lattice->channelCentroid(corners));
    }
  }
}

unsigned int
HexagonalSubchannelBin::bin(const Point & p) const
{
  if (_pin_centered_bins)
    return _hex_lattice->pinIndex(p);
  else
    return _hex_lattice->channelIndex(p);
}

unsigned int
HexagonalSubchannelBin::num_bins() const
{
  if (_pin_centered_bins)
    return _hex_lattice->nPins() + 1;
  else
    return _hex_lattice->nChannels();
}
