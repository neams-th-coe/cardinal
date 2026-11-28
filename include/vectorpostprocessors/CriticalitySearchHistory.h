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

#include "GeneralVectorPostprocessor.h"
#include "OpenMCBase.h"

/**
 * Reports every value tried in the most recent criticality search, along with the
 * k eigenvalue (mean and standard deviation) and reactivity computed for each.
 */
class CriticalitySearchHistory : public GeneralVectorPostprocessor, public OpenMCBase
{
public:
  static InputParameters validParams();

  CriticalitySearchHistory(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// Values of the searched quantity, in the order tried
  VectorPostprocessorValue & _input;

  /// Mean k for each value tried
  VectorPostprocessorValue & _k;

  /// Standard deviation of k for each value tried
  VectorPostprocessorValue & _k_std_dev;

  /// Reactivity (k - 1) / k for each value tried, in pcm
  VectorPostprocessorValue & _reactivity;

  /// Standard deviation of the reactivity for each value tried, in pcm
  VectorPostprocessorValue & _reactivity_std_dev;
};
