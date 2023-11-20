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

#ifdef ENABLE_OPENMC_COUPLING

#include "Control.h"

class OpenMCNuclideDensitiesControl : public Control
{
public:
  static InputParameters validParams();

  OpenMCNuclideDensitiesControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// The controllable parameter prefix for the associated OpenMCNuclideDensities
  const std::string _controllable_prefix;
  /// The provided names
  const std::vector<std::vector<std::string>> & _names;
  /// The provided densities
  const std::vector<std::vector<Real>> & _densities;

private:
  /// The index for the current execution (used for indexing into names and densities)
  unsigned int _current_execution;
};

#endif
