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

#include "GeneralPostprocessor.h"

#include "OpenMCBase.h"
#include "CardinalEnums.h"

/**
 * Get the k-effective eigenvalue computed by OpenMC.
 */
class KEigenvalue : public GeneralPostprocessor, public OpenMCBase
{
public:
  static InputParameters validParams();

  KEigenvalue(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() const override;

protected:
  /**
   * A function which computes the relative error of k_{eff}.
   * @return the relative error of the k-eigenvalue
   */
  Real kRelativeError() const;

  /**
   * Type of k-effective value to report. Options:
   * collision, absorption, tracklength, and combined (default).
   *
   * The combined k-effective estimate is a minimum variance estimate
   * of k-effective based on a linear combination of the collision, absorption,
   * and tracklength estimates.
   */
  const eigenvalue::EigenvalueEnum _type;

  /// The value of the kinetics parameter to output.
  const statistics::OutputEnum _output;
};
