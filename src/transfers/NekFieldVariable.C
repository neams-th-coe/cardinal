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

#include "NekFieldVariable.h"

registerMooseObject("CardinalApp", NekFieldVariable);

InputParameters
NekFieldVariable::validParams()
{
  auto params = FieldTransferBase::validParams();
  params.addParam<MooseEnum>(
      "field",
      getNekOutputEnum(),
      "NekRS field variable to read/write; defaults to the name of the object");
  params.addClassDescription("Reads/writes volumetric field data between NekRS and MOOSE.");
  return params;
}

NekFieldVariable::NekFieldVariable(const InputParameters & parameters)
  : FieldTransferBase(parameters)
{
  if (_direction == "from_nek")
    addExternalVariable(_variable);

  if (_direction == "to_nek")
    paramError("direction",
               "The NekFieldVariable currently only supports transfers 'from_nek'; contact the "
               "Cardinal developer team if you require writing of NekRS field variables.");

  if (isParamValid("field"))
    _field = convertToFieldEnum(getParam<MooseEnum>("field"));
  else
  {
    // try a reasonable default by seeing if the object name matches a valid enumeration
    auto enums = getNekOutputEnum().getNames();
    auto obj_name = name();
    std::transform(obj_name.begin(), obj_name.end(), obj_name.begin(), ::toupper);
    bool found_name = std::find(enums.begin(), enums.end(), obj_name) != enums.end();
    if (found_name)
      _field = convertToFieldEnum(obj_name);
    else
      paramError("field",
                 "We tried to choose a default 'field' as '" + name() +
                     "', but this value is not an option in the 'field' enumeration. Please "
                     "provide the 'field' parameter.");
  }

  _external_data = (double *)calloc(_nek_problem.nPoints(), sizeof(double));
}

NekFieldVariable::~NekFieldVariable() { freePointer(_external_data); }

field::NekFieldEnum
NekFieldVariable::convertToFieldEnum(const std::string name) const
{
  auto lowercase = name;
  std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
  if (lowercase == "velocity_x")
    return field::velocity_x;
  if (lowercase == "velocity_y")
    return field::velocity_y;
  if (lowercase == "velocity_z")
    return field::velocity_z;
  if (lowercase == "temperature")
    return field::temperature;
  if (lowercase == "pressure")
    return field::pressure;
  if (lowercase == "scalar01")
    return field::scalar01;
  if (lowercase == "scalar02")
    return field::scalar02;
  if (lowercase == "scalar03")
    return field::scalar03;

  mooseError("Unhandled NekFieldEnum in NekFieldVariable!");
}

void
NekFieldVariable::readDataFromNek()
{
  if (!_nek_mesh->volume())
    _nek_problem.boundarySolution(_field, _external_data);
  else
    _nek_problem.volumeSolution(_field, _external_data);

  fillAuxVariable(_variable_number[_variable], _external_data);
}

#endif
