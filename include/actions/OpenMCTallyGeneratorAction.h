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
 * User object to modify the nuclides in an OpenMC tally.
 */
class OpenMCTallyGeneratorAction : public Action
{
public:
  static InputParameters validParams();

  OpenMCTallyGeneratorAction(const InputParameters & parameters);

  virtual void act();

protected:
  std::vector<std::string> _ids;
  bool _nuclides_uo {true};
};
