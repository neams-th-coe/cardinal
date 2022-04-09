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

#ifdef ENABLE_NEK_COUPLING

#include "NekUserObject.h"

InputParameters
NekUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  return params;
}

NekUserObject::NekUserObject(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters)
{
  _nek_problem = dynamic_cast<const NekRSProblemBase *>(&_fe_problem);
  if (!_nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped Nek cases!\n"
               "You need to change the problem type from '" +
               _fe_problem.type() + "'" + extra_help +
               " to a Nek-wrapped problem.\n\n"
               "options: 'NekRSProblem', 'NekRSStandaloneProblem'");
  }

  _fixed_mesh = !(_nek_problem->movingMesh());
}

void
NekUserObject::checkValidField(const field::NekFieldEnum & field) const
{
  if (!nekrs::hasTemperatureVariable() && field == field::temperature)
    mooseError("This user object cannot set 'field = temperature' "
               "because your Nek case files do not have a temperature variable!");

  if (!nekrs::hasScalarVariable(1) && field == field::scalar01)
    mooseError("This user object cannot set 'field = scalar01' "
               "because your Nek case files do not have a scalar01 variable!");

  if (!nekrs::hasScalarVariable(2) && field == field::scalar02)
    mooseError("This user object cannot set 'field = scalar02' "
               "because your Nek case files do not have a scalar02 variable!");

  if (!nekrs::hasScalarVariable(3) && field == field::scalar03)
    mooseError("This user object cannot set 'field = scalar03' "
               "because your Nek case files do not have a scalar03 variable!");
}

#endif
