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
    mooseError("UserObject with name '" + name() + "' can only be used with wrapped Nek cases!\n"
      "You need to change the problem type from '" + _fe_problem.type() + "'" + extra_help +" to a Nek-wrapped problem.\n\n"
      "options: 'NekRSProblem', 'NekRSStandaloneProblem'");
  }
}

void
NekUserObject::checkValidField(const field::NekFieldEnum & field) const
{
  if (!nekrs::hasTemperatureVariable() && field == field::temperature)
    mooseError("UserObject with name '" + name() + "' cannot set 'field = temperature' "
      "because your Nek case files do not have a temperature variable!");
}
