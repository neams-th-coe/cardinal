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

#include "MooseEnum.h"
#include "MooseUtils.h"

#include "OpenMCCellTransformBase.h"
#include "OpenMCBase.h"
#include "UserErrorChecking.h"

registerMooseObject("CardinalApp", OpenMCCellTransformBase);

InputParameters
OpenMCCellTransformBase::validParams()
{
  auto params = emptyInputParameters();

  params.addRequiredParam<std::vector<int32_t>>(
      "cell_ids", "List of OpenMC cell IDs whose filled universes will be transformed.");

  return params;
}

const MooseEnum OpenMCCellTransformBase::transform_type("translation rotation", "translation");
const std::array<std::string, 3> OpenMCCellTransformBase::rotation_vector_symbols{"φ", "θ", "ψ"};
const std::string OpenMCCellTransformBase::rotation_vector_symbols_list =
    OpenMCCellTransformBase::rotation_vector_symbols[0] + ", " +
    OpenMCCellTransformBase::rotation_vector_symbols[1] + ", " +
    OpenMCCellTransformBase::rotation_vector_symbols[2];

OpenMCCellTransformBase::OpenMCCellTransformBase(const MooseObject & moose_object)
  : _cell_ids(moose_object.getParam<std::vector<int32_t>>("cell_ids").begin(),
              moose_object.getParam<std::vector<int32_t>>("cell_ids").end()),
    _moose_object(moose_object),
    _openmc_problem(getOpenMCProblem(moose_object))
{
  if (_cell_ids.empty())
    moose_object.paramError("cell_ids", "At least one OpenMC cell ID must be provided.");
  if (_cell_ids.size() != _moose_object.getParam<std::vector<int32_t>>("cell_ids").size())
    moose_object.paramError("cell_ids",
                            "Duplicate OpenMC cell IDs were detected. Provide each ID only once.");
}

void
OpenMCCellTransformBase::transform(const MooseEnum & transform_type, const Point & transform_vector)
{
  std::array<double, 3> vec{transform_vector(0), transform_vector(1), transform_vector(2)};

  if (transform_type == "translation")
    for (const auto i : index_range(vec))
      vec[i] *= _openmc_problem.scaling();

  for (const auto & cell_id : _cell_ids)
  {
    int32_t index = -1;

    int err = openmc_get_cell_index(cell_id, &index);
    catchOpenMCError(err, "find OpenMC cell with ID " + std::to_string(cell_id));

    if (transform_type == "translation")
    {
      // If a user tried to apply translation on a cell that doesn't contain a filled universe,
      // OpenMC will return an error.
      err = openmc_cell_set_translation(index, vec.data());
      _moose_object._console << "Setting OpenMC cell translation for cell with ID " << cell_id
                             << " to (" << vec[0] << ", " << vec[1] << ", " << vec[2] << ") cm."
                             << std::endl;
    }
    else if (transform_type == "rotation")
    {
      for (const auto i : index_range(vec))
        if (vec[i] < 0 || vec[i] >= 360)
          _moose_object.mooseError("Rotation transformation angle ",
                                   rotation_vector_symbols[i],
                                   " must be in the range [0, 360) degrees. Got invalid ",
                                   rotation_vector_symbols[i],
                                   " = ",
                                   vec[i],
                                   " degrees.");
      // If a user tried to apply rotation on a cell that doesn't contain a filled universe,
      // OpenMC will return an error.
      err = openmc_cell_set_rotation(index, vec.data(), 3);
      _moose_object._console << "Setting OpenMC cell rotation for cell with ID " << cell_id
                             << " to (" << vec[0] << ", " << vec[1] << ", " << vec[2]
                             << ") degrees." << std::endl;
    }
    else
      _moose_object.mooseError("In transformation, unknown transformation type: " + transform_type);

    catchOpenMCError(err, "transform OpenMC cell OpenMC cell with ID " + std::to_string(cell_id));
  }
}

#endif
