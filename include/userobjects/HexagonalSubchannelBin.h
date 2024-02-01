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

#include "SpatialBinUserObject.h"
#include "HexagonalLatticeUtils.h"

/**
 * Class that bins spatial coordinates into a hexagonal subchannel discretization
 * with unique bins for each channel.
 */
class HexagonalSubchannelBin : public SpatialBinUserObject
{
public:
  static InputParameters validParams();

  HexagonalSubchannelBin(const InputParameters & parameters);

  virtual unsigned int bin(const Point & p) const override;

  virtual unsigned int num_bins() const override;

protected:
  /// Bundle pitch
  const Real & _bundle_pitch;

  /// Pin pitch
  const Real & _pin_pitch;

  /// Pin diameter
  const Real & _pin_diameter;

  /// Total number of rings of pins
  const unsigned int & _n_rings;

  /// Vertical axis of the bundle along which the pins are aligned
  const unsigned int _axis;

  /// Whether the bins should be pin-centered (as opposed to channel-centered)
  const bool & _pin_centered_bins;

  /// Underlying utility providing hexagonal lattice capabilities
  std::unique_ptr<HexagonalLatticeUtils> _hex_lattice;
};
