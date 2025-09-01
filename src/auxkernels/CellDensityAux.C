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

#include "CellDensityAux.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", CellDensityAux);

InputParameters
CellDensityAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("OpenMC fluid density (kg/m$^3$), mapped to each MOOSE element");
  return params;
}

CellDensityAux::CellDensityAux(const InputParameters & parameters) : OpenMCAuxKernel(parameters) {}

Real
CellDensityAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a density of -1; otherwise, we would
  // get an error in the call to cellCouplingFields, since it relies on the
  // OpenMCCellAverageProblem::_cell_to_elem std::map that wouldn't have an entry that corresponds
  // to an unmapped cell
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());

  // we only extract the material information for density feedback cells, because otherwise we don't
  // need to know the material info. So, set a value of -1 for non-density feedback cells.
  if (!_openmc_problem->hasDensityFeedback(cell_info))
    return OpenMCCellAverageProblem::UNMAPPED;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);

  // if the material is void, return -1
  if (_openmc_problem->materialID(index) == -1)
    return OpenMCCellAverageProblem::UNMAPPED;

  // Fetch the density.
  double density;
  int err = openmc_cell_get_density(cell_info.first, &cell_info.second, &density);

  if (err)
    mooseError("In attempting to get the density for " + _openmc_problem->printCell(cell_info) +
               ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return density / _openmc_problem->densityConversionFactor();
}

#endif
