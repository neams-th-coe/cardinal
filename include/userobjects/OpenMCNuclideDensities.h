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

#include "OpenMCUserObject.h"

/**
 * User object to modify the nuclide densities in an OpenMC material.
 */
class OpenMCNuclideDensities : public OpenMCUserObject
{
public:
  static InputParameters validParams();

  OpenMCNuclideDensities(const InputParameters & parameters);

  /// Instead, we want to have a separate method that we can call from the OpenMC problem
  virtual void setValue();

protected:
  /// The material ID
  const int32_t & _material_id;

  /// The material index
  int32_t _material_index;

  /// Nuclide names
  const std::vector<std::string> & _names;

  /// Nuclide densities
  const std::vector<double> & _densities;
};
