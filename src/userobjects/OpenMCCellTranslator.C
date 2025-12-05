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

#include "OpenMCCellTranslator.h"
#include "CardinalAppTypes.h"
#include <string>

registerMooseObject("CardinalApp", OpenMCCellTranslator);

InputParameters
OpenMCCellTranslator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();

  params.addRequiredParam<std::vector<int32_t>>(
      "cell_ids",
      "List of OpenMC cell IDs whose filled universes will be translated.");

  params.addRequiredParam<PostprocessorName>(
      "x_translation",
      "Postprocessor giving x-translation in mesh units.");
  params.addRequiredParam<PostprocessorName>(
      "y_translation",
      "Postprocessor giving y-translation in mesh units.");
  params.addRequiredParam<PostprocessorName>(
      "z_translation",
      "Postprocessor giving z-translation in mesh units.");

  params.addClassDescription(
      "UserObject that sets the translation vector on one or more OpenMC cells. "
      "The translation is driven by three MOOSE postprocessors (x, y, z)");

  return params;
}

OpenMCCellTranslator::OpenMCCellTranslator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _cell_ids(getParam<std::vector<int32_t>>("cell_ids")),
    _dx_pp(getPostprocessorValue("x_translation")),
    _dy_pp(getPostprocessorValue("y_translation")),
    _dz_pp(getPostprocessorValue("z_translation")),
    _scaling(1.0)
{
  if (_cell_ids.empty())
    paramError("cell_ids", "At least one OpenMC cell ID must be provided.");

  if (_openmc_problem)
    _scaling = _openmc_problem->scaling();
}

void
OpenMCCellTranslator::execute()
{
  const Real dx = _scaling * _dx_pp;
  const Real dy = _scaling * _dy_pp;
  const Real dz = _scaling * _dz_pp;

  double translation[3] = {dx, dy, dz};

  _console << "Setting OpenMC cell translations for " << _cell_ids.size()
           << " cell(s) to (" << dx << ", " << dy << ", " << dz
           << ") in OpenMC geometry units." << std::endl;

  for (const auto & cell_id : _cell_ids)
  {
    int32_t index = -1;

    int err = openmc_get_cell_index(cell_id, &index);
    if (err)
    {
      mooseError("In attempting to find OpenMC cell with ID " +
                        std::to_string(cell_id) +
                        ", OpenMC reported:\n\n" +
                        std::string(openmc_err_msg));
    }

    err = openmc_cell_set_translation(index, translation);
    if (err)
    {
      mooseError("In attempting to set translation for OpenMC cell with ID " +
                        std::to_string(cell_id) +
                        ", OpenMC reported:\n\n" +
                        std::string(openmc_err_msg));
    }
  }
}

#endif
