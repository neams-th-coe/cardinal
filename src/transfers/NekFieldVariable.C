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
      getNekFieldEnum(),
      "NekRS field variable to read/write; defaults to the name of the object");
  params.addClassDescription("Reads/writes volumetric field data between NekRS and MOOSE.");
  return params;
}

NekFieldVariable::NekFieldVariable(const InputParameters & parameters)
  : FieldTransferBase(parameters)
{
  if (_direction == "from_nek")
    addExternalVariable(_variable);
  else
  {
    if (_usrwrk_slot.size() > 1)
      paramError("usrwrk_slot",
                 "'usrwrk_slot' must be of length 1 for field transfers to_nek; you have entered "
                 "a vector of length " +
                     Moose::stringify(_usrwrk_slot.size()));

    addExternalVariable(_usrwrk_slot[0], _variable);

    // we don't impose any requirements on boundary conditions on the NekRS side, because this data
    // being sent to NekRS doesn't necessarily get used in a boundary condition. It could get used
    // in a source term, for instance.
  }

  if (isParamValid("field"))
    _field = getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>();
  else
  {
    // try a reasonable default by seeing if the object name matches a valid enumeration
    auto enums = getNekFieldEnum().getNames();
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

  if (_direction == "to_nek")
  {
    switch (_field)
    {
      case field::temperature:
        indices.temperature = _usrwrk_slot[0] * nekrs::fieldOffset();
        break;
      default:
        paramError("field",
                   "NekFieldVariable currently only supports transfers 'to_nek' for 'temperature'. "
                   "Please contact the Cardinal developer team if you require writing of other "
                   "NekRS field variables.");
    }
  }

  if (_field == field::velocity_component)
    paramError("field",
               "'velocity_component' is not yet supported; if you need velocity dotted with a "
               "specific direction, extract the three components of velocity and perform the "
               "postprocessing operation using a ParsedAux. If this is hindering your workflow, "
               "please contact the Cardinal developer team.");
}

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
  if (lowercase == "velocity")
    return field::velocity;
  if (lowercase == "velocity_component")
    return field::velocity_component;
  if (lowercase == "velocity_x_squared")
    return field::velocity_x_squared;
  if (lowercase == "velocity_y_squared")
    return field::velocity_y_squared;
  if (lowercase == "velocity_z_squared")
    return field::velocity_z_squared;
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
  if (lowercase == "unity")
    return field::unity;
  if (lowercase == "usrwrk00")
    return field::usrwrk00;
  if (lowercase == "usrwrk01")
    return field::usrwrk01;
  if (lowercase == "usrwrk02")
    return field::usrwrk02;

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

void
NekFieldVariable::sendDataToNek()
{
  _console << "Sending " << _variable << " to NekRS..." << std::endl;
  auto d = nekrs::nondimensionalDivisor(_field);
  auto a = nekrs::nondimensionalAdditive(_field);

  if (!_nek_mesh->volume())
  {
    for (unsigned int e = 0; e < _nek_mesh->numSurfaceElems(); e++)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current
      // process
      if (nekrs::commRank() != _nek_mesh->boundaryCoupling().processor_id(e))
        continue;

      _nek_problem.mapFaceDataToNekFace(e, _variable_number[_variable], d, a, &_v_face);
      _nek_problem.writeBoundarySolution(e, _field, _v_face);
    }
  }
  else
  {
    for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); ++e)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current
      // process
      if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
        continue;

      _nek_problem.mapVolumeDataToNekVolume(e, _variable_number[_variable], d, a, &_v_elem);
      _nek_problem.writeVolumeSolution(e, _field, _v_elem);
    }
  }
}

#endif
