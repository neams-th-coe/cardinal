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

#include "MooseObject.h"
#include "OpenMCBase.h"
#include "CardinalEnums.h"

/**
 * Applies a criticality search for each OpenMC solve. The critical configuration
 * is then used for the multiphysics coupling.
 */
class CriticalitySearchBase : public MooseObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  CriticalitySearchBase(const InputParameters & parameters);

  /**
   * Modify the OpenMC model
   * @param[in] input value to apply to the new model; interpretation depends on derived classes
   */
  virtual void updateOpenMCModel(const Real & input) = 0;

  /// Use Brent's method to search for criticality
  virtual void searchForCriticality();

protected:
  /// The quantity being varied in the search for criticality, for console prints
  virtual std::string quantity() const = 0;

  /// Assumed units in the input quantities
  virtual std::string units() const = 0;

  /// Maximum range of value to explore
  const Real & _maximum;

  /// Minimum range of value to explore
  const Real & _minimum;

  /// Absolute tolerance for finding a critical configuration
  const Real & _tolerance;

  /// Estimator to use for k
  const eigenvalue::EigenvalueEnum _estimator;

  /// Values used in search
  std::vector<Real> _inputs;

  /// Values obtained in search
  std::vector<Real> _k_values;

  /// Standard deviation values obtained in search
  std::vector<Real> _k_std_dev_values;

  /// Postprocessor that holds the result of the criticality search
  const std::string _pp_name = "critical_value";
};
