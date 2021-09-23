#include "HexagonalSubchannelBin.h"

registerMooseObject("CardinalApp", HexagonalSubchannelBin);

InputParameters
HexagonalSubchannelBin::validParams()
{
  InputParameters params = SpatialBinUserObject::validParams();
  params.addRequiredRangeCheckedParam<Real>("bundle_pitch", "bundle_pitch > 0",
    "Bundle pitch, or flat-to-flat distance across bundle");
  params.addRequiredRangeCheckedParam<Real>("pin_pitch", "pin_pitch > 0",
    "Pin pitch, or distance between pin centers");
  params.addRequiredRangeCheckedParam<Real>("pin_diameter", "pin_diameter > 0",
    "Pin outer diameter");
  params.addRequiredRangeCheckedParam<unsigned int>("n_rings", "n_rings >= 1",
    "Number of pin rings, including the centermost pin as a 'ring'");

  MooseEnum directions("x y z", "z");
  params.addParam<MooseEnum>("axis", directions,
    "vertical axis of the reactor (x, y, or z) along which pins are aligned");
  params.addClassDescription("Creates a unique spatial bin for each subchannel in a hexagonal lattice");
  return params;
}

HexagonalSubchannelBin::HexagonalSubchannelBin(const InputParameters & parameters)
  : SpatialBinUserObject(parameters),
  _bundle_pitch(getParam<Real>("bundle_pitch")),
  _pin_pitch(getParam<Real>("pin_pitch")),
  _pin_diameter(getParam<Real>("pin_diameter")),
  _n_rings(getParam<unsigned int>("n_rings")),
  _axis(parameters.get<MooseEnum>("axis"))
{
  _hex_lattice.reset(new HexagonalLatticeUtility(_bundle_pitch, _pin_pitch, _pin_diameter,
    0.0 /* wire diameter, unused */, 1.0 /* wire pitch, unused */, _n_rings, _axis));
}

const unsigned int
HexagonalSubchannelBin::bin(const Point & p) const
{
  return _hex_lattice->channelIndex(p);
}

const unsigned int
HexagonalSubchannelBin::num_bins() const
{
  return _hex_lattice->nChannels();
}
