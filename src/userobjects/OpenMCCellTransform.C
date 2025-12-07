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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCCellTransform.h"
#include "CardinalAppTypes.h"
#include <string>

registerMooseObject("CardinalApp", OpenMCCellTransform);

InputParameters
OpenMCCellTransform::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();

  params.addRequiredParam<std::vector<int32_t>>(
      "cell_ids", "List of OpenMC cell IDs whose filled universes will be transformed.");

  MooseEnum transform_type("translation rotation", "translation");
  params.addParam<MooseEnum>(
      "transform_type",
      transform_type,
      "Type of transform to apply: 'translation' (dx,dy,dz) or 'rotation' (phi,theta,psi) in degrees.");

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "transform_array",
      "An array of three postprocessors. For translation this array expects (dx, dy, dz) in mesh units."
      "For rotation this array expects 'φ, θ, ψ' in degrees.");

  params.addClassDescription(
      "UserObject that applies either translation or rotation on one or more OpenMC cells. "
      "The transform is driven by three MOOSE postprocessors (x, y, z)");

  return params;
}

OpenMCCellTransform::OpenMCCellTransform(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _cell_ids(getParam<std::vector<int32_t>>("cell_ids")),
    _transform_type(getParam<MooseEnum>("transform_type")),
    _scaling(1.0)
{
  if (_cell_ids.empty())
    paramError("cell_ids", "At least one OpenMC cell ID must be provided.");

  const auto & t = getParam<std::vector<PostprocessorName>>("transform_array");
  if (t.size() != 3)
    paramError("transform_array", "Provide exactly 3 postprocessors: 'dx dy dz' in mesh units for translation"
               "transform or 'φ, θ, ψ' in degrees for rotation transform.");

  _t0_pp = &GeneralUserObject::getPostprocessorValue("transform_array", 0);
  _t1_pp = &GeneralUserObject::getPostprocessorValue("transform_array", 1);
  _t2_pp = &GeneralUserObject::getPostprocessorValue("transform_array", 2);

  if (_openmc_problem)
    _scaling = _openmc_problem->scaling();
}

void
OpenMCCellTransform::execute()
{
  const Real a0 = *_t0_pp;
  const Real a1 = *_t1_pp;
  const Real a2 = *_t2_pp;

  double vec[3];

  if (_transform_type == "translation")
  {
    vec[0] = _scaling * a0;
    vec[1] = _scaling * a1;
    vec[2] = _scaling * a2;

    _console << "Setting OpenMC cell translations for " << _cell_ids.size() << " cell(s) to ("
             << vec[0] << ", " << vec[1] << ", " << vec[2] << ") cm."
             << std::endl;
  }
  else if (_transform_type == "rotation")
  {
    vec[0] = a0;
    vec[1] = a1;
    vec[2] = a2;

    _console << "Setting OpenMC cell rotations for " << _cell_ids.size() << " cell(s) to ("
             << vec[0] << ", " << vec[1] << ", " << vec[2] << ") degrees." << std::endl;
  }
  else
    mooseError("Unhandled transform_type: " + _transform_type);

  for (const auto & cell_id : _cell_ids)
  {
    int32_t index = -1;

    int err = openmc_get_cell_index(cell_id, &index);
    if (err)
      mooseError("In attempting to find OpenMC cell with ID " + std::to_string(cell_id) +
                 ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

    if (_transform_type == "translation")
      err = openmc_cell_set_translation(index, vec);
    else
      err = openmc_cell_set_rotation(index, vec, 3);

    if (err)
      mooseError("In attempting to ", (_transform_type == "translation" ? "translate" : "rotate"),
                 " OpenMC cell with ID ", cell_id, ", OpenMC reported:\n\n", openmc_err_msg);

  }
}

#endif
