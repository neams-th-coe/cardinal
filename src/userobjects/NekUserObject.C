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
  params.addParam<unsigned int>("interval", 1,
    "Frequency (in number of time steps) with which to execute this user object; "
    "because Nek uses very small time steps, you need many time steps to reach "
    "steady state, and user objects can be expensive and not necessary to evaluate "
    "on every single time step. NOTE: you probably want to match this 'interval' "
    "in the Output");
  return params;
}

NekUserObject::NekUserObject(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _interval(getParam<unsigned int>("interval"))
{
  _nek_problem = dynamic_cast<const NekRSProblemBase *>(&_fe_problem);
  if (!_nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped Nek cases!\n"
               "You need to change the problem type from '" +
               _fe_problem.type() + "'" + extra_help +
               " to a Nek-wrapped problem.\n\n"
               "options: 'NekRSProblem', 'NekRSSeparateDomainProblem', 'NekRSStandaloneProblem'");
  }

  _fixed_mesh = !(_nek_problem->hasMovingNekMesh());
}

void
NekUserObject::execute()
{
  if (_fe_problem.timeStep() % _interval == 0)
    executeUserObject();
}

#endif
