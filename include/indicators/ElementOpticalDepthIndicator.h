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

#include "OpenMCIndicator.h"

/**
 * An Indicator which returns an estimate of the optical depth experienced by photons/neutrons which
 * traverse the element.
 */
class ElementOpticalDepthIndicator : public OpenMCIndicator
{
public:
  static InputParameters validParams();

  ElementOpticalDepthIndicator(const InputParameters & parameters);

  virtual void computeIndicator() override;

protected:
  /// The type of element length to use for estimating the optical depth.
  enum class HType
  {
    Min = 0,
    Max = 1,
    CubeRoot = 2
  } _h_type;

  /**
   * The variables containing the reaction rate. This needs to be a vector because the reaction rate
   * score may have filters applied, and so we need to sum the reaction rate over all filter bins.
   */
  std::vector<const VariableValue *> _rxn_rates;

  /**
   * The variables containing the scalar flux. This needs to be a vector because the scalar flux
   * score may have filters applied, and so we need to sum the scalar fluxes over all filter bins.
   */
  std::vector<const VariableValue *> _scalar_fluxes;

  /// Whether or not the optical depth should be inverted or not.
  const bool _invert;
};
