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

#include "Action.h"

/**
 * Initialize OpenMC.
 */
class OpenMCInitAction : public Action
{
public:
  OpenMCInitAction(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void act() override;

protected:
  /// Check if OpenMCCellAverageProblem is present and return xml_directory if it is the case
  bool get_openmc_problem_type_xml_directory(std::string & xml_directory) const;

  /// Call the OpenMC initialization handle
  void initOpenMC(const std::string & xml_directory);
};
