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

registerMooseObject("CardinalApp", OpenMCCellTransform);

InputParameters
OpenMCCellTransform::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();

  params.addRequiredParam<std::vector<int32_t>>(
      "cell_ids", "List of OpenMC cell IDs whose filled universes will be transformed.");

  MooseEnum transform_type("translation rotation", "translation");
  params.addParam<MooseEnum>("transform_type",
                             transform_type,
                             "Type of transform to apply: 'translation' (dx,dy,dz) or 'rotation'"
                             "(φ, θ, ψ) in degrees.");

  params.addRequiredParam<std::vector<PostprocessorName>>(
      "vector_value",
      "An array of three values/postprocessors. For translation this array expects (dx, dy, dz) in mesh"
      " units. For rotation this array expects 'φ, θ, ψ' in degrees.");

  params.addClassDescription(
      "UserObject that applies either translation or rotation on one or more OpenMC cells. "
      "The transform is driven by a transform array of three MOOSE postprocessors");

  return params;
}

OpenMCCellTransform::OpenMCCellTransform(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _cell_ids(getParam<std::vector<int32_t>>("cell_ids")),
    _transform_type(getParam<MooseEnum>("transform_type"))
{
  if (_cell_ids.empty())
    paramError("cell_ids", "At least one OpenMC cell ID must be provided.");

  const auto & t = getParam<std::vector<PostprocessorName>>("vector_value");
  if (t.size() != 3)
    paramError("vector_value",
               "Provide exactly 3 values/postprocessors: 'dx dy dz' in mesh units for translation"
               "transform or 'φ, θ, ψ' in degrees for rotation transform.");

  _t0_pp = &getPostprocessorValue("vector_value", 0);
  _t1_pp = &getPostprocessorValue("vector_value", 1);
  _t2_pp = &getPostprocessorValue("vector_value", 2);

}

void
OpenMCCellTransform::execute()
{
  double vec[3];
  vec[0] = *_t0_pp;
  vec[1] = *_t1_pp;
  vec[2] = *_t2_pp;

  if (_transform_type == "translation")
  {
    vec[0] *= _openmc_problem->scaling();
    vec[1] *= _openmc_problem->scaling();
    vec[2] *= _openmc_problem->scaling();
  }

  for (const auto & cell_id : _cell_ids)
  {
    int32_t index = -1;

    int err = openmc_get_cell_index(cell_id, &index);
    _openmc_problem->catchOpenMCError(err, "In attempting to find OpenMC cell with ID " + std::to_string(cell_id) +
                 ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

    if (_transform_type == "translation")
    {
      err = openmc_cell_set_translation(index, vec);
      _console << "Setting OpenMC cell translations for cell with ID" + std::to_string(cell_id) + "to ("
             << vec[0] << ", " << vec[1] << ", " << vec[2] << ") cm." << std::endl;
    }
    else if (_transform_type == "rotation")
    {
      err = openmc_cell_set_rotation(index, vec, 3);
      _console << "Setting OpenMC cell rotations for cell with ID" + std::to_string(cell_id) + "to ("
             << vec[0] << ", " << vec[1] << ", " << vec[2] << ") degrees." << std::endl;
    }
    else
      mooseError("Unhandled transform_type: " + _transform_type);

    _openmc_problem->catchOpenMCError(err, "In attempting to transform OpenMC cell OpenMC cell with ID " + std::to_string(cell_id) +
                 ", OpenMC reported:\n\n" + std::string(openmc_err_msg));
  }
}

#endif
