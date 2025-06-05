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

#include "NekRSProblemBase.h"
#include "NekRSMesh.h"
#include "FieldTransferBase.h"
#include "UserErrorChecking.h"
#include "AuxiliarySystem.h"

registerMooseObject("CardinalApp", FieldTransferBase);

InputParameters
FieldTransferBase::validParams()
{
  auto params = MooseObject::validParams();
  MooseEnum direction("to_nek from_nek");
  params.addRequiredParam<MooseEnum>("direction", direction, "Direction in which to send data; 'from_nek' will read from the 'field' variable in NekRS and write into the 'variable'; 'to_nek' will read from the 'variable' and write into the 'field' variable in NekRS");

  params.addParam<std::string>("variable", "Variable to fill with NekRS 'field' data, if 'direction = from_nek'; defaults to name of this object if not provided");
  params.addParam<std::string>("source_variable", "Variable to read from to populate the NekRS 'field', if 'direction = to_nek'; defaults to the name of this object if not provided");

  params.addPrivateParam<NekRSProblemBase *>("_nek_problem");
  params.registerBase("FieldTransfer");
  params.registerSystemAttributeName("FieldTransfer");

  return params;
}

FieldTransferBase::FieldTransferBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _nek_problem(*getParam<NekRSProblemBase *>("_nek_problem")),
    _direction(getParam<MooseEnum>("direction"))
{
  _nek_mesh = dynamic_cast<NekRSMesh *>(&(getMooseApp().feProblem().mesh()));

  if (!_nek_mesh)
    mooseError("Mesh for must be of type 'NekRSMesh'");

  // when writing data into Nek, we require a variable to read from; if not
  // provided, then choose the object name as default
  if (_direction == "to_nek")
  {
    checkUnusedParam(parameters, "variable", "writing data 'to_nek'");
    if (isParamValid("source_variable"))
      _variable = getParam<std::string>("source_variable");
    else
      _variable = name();
  }
  else if (_direction == "from_nek")
  {
    checkUnusedParam(parameters, "source_variable", "reading data 'from_nek'");
    if (isParamValid("variable"))
      _variable = getParam<std::string>("variable");
    else
      _variable = name();
  }
  else
    mooseError("Unhandled direction enum in FieldTransferBase!");

  addExternalVariable(_variable);
}

void
FieldTransferBase::addExternalVariable(const std::string name)
{
  auto var_params = _nek_problem.getExternalVariableParameters();
  _nek_problem.checkDuplicateVariableName(name);
  _nek_problem.addAuxVariable("MooseVariable", name, var_params);
}

void
FieldTransferBase::addExternalPostprocessor(const std::string name, const Real initial)
{
  auto pp_params = _factory.getValidParams("Receiver");
  pp_params.set<Real>("default") = initial;
  _nek_problem.addPostprocessor("Receiver", name, pp_params);
  _variable_number = _nek_problem.getAuxiliarySystem().getFieldVariable<Real>(0, _variable).number();
}
#endif
