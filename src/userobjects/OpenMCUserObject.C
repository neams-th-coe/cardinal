/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2024 UChicago Argonne, LLC                  */
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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCUserObject.h"
#include "OpenMCProblemBase.h"

InputParameters
OpenMCUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

OpenMCUserObject::OpenMCUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _first_execution(true)
{
  if (!openmc_problem())
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped OpenMC cases! "
               "You need to change the\nproblem type from '" +
               _fe_problem.type() + "'" + extra_help + " to OpenMCCellAverageProblem.");
  }
}

void
OpenMCUserObject::execute()
{
  if (!_first_execution)
    return;
  initialize();
  _first_execution = false;
}

const OpenMCProblemBase *
OpenMCUserObject::openmc_problem() const
{
  return dynamic_cast<const OpenMCProblemBase *>(&_fe_problem);
}
#endif
