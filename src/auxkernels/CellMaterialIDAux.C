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

#include "CellMaterialIDAux.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", CellMaterialIDAux);

InputParameters
CellMaterialIDAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("Display the OpenMC fluid material ID mapped to each MOOSE element");
  return params;
}

CellMaterialIDAux::CellMaterialIDAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters)
{
}

Real
CellMaterialIDAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a cell ID of -1; otherwise, we would
  // get an error in the call to cellCouplingFields and cellToMaterialIndex, since these
  // rely on a valid cell instance, index pair being passed to OpenMC's C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  // we only extract the material information for fluid cells, because otherwise we don't
  // need to know the material info. So, set a value of -1 for non-fluid cells.
  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());
  if (!_openmc_problem->hasDensityFeedback(cell_info))
    return -1;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);
  int32_t id = _openmc_problem->materialID(index);

  return id;
}

#endif
