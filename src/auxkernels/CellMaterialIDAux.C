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
  params.addClassDescription("OpenMC material ID, mapped to each MOOSE element");
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
  // get an error in the call to cellCouplingFields, since these
  // rely on a valid cell instance, index pair being passed to OpenMC's C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());

  if (!_openmc_problem->hasDensityFeedback(cell_info))
    return -1;

  // if the cell which maps to this element contains more than one cell within it (e.g. if filled
  // by a universe or lattice), then we cannot return a single value from this function. We check
  // if there are multiple cell IDs; if this does not fail, then we still need to check the number
  // of instances of the single-ID fill.
  const auto & contained_cells = _openmc_problem->containedMaterialCells(cell_info);
  bool multiple_contained_cells = contained_cells.size() > 1;
  if (!multiple_contained_cells)
    multiple_contained_cells = contained_cells[0].begin()->second.size() > 1;

  // we could technically have a situation where you build a lattice of single-material universes,
  // which would trigger this error message. I opted for an error here because it's likely not worth
  // the expense to form a list of all the materials filling the given cell to see if we're actually
  // in a situation where we have multiple MATERIALs in those cells, vs. multiple cells all filled by
  // the same material.
  if (multiple_contained_cells)
    mooseError("Element ", this->_current_elem->id(), " maps to OpenMC cell ", printCell(cell_info), " which contains more than one material-fill cell (for instance, by being filled by a universe or lattice). Therefore, we cannot easily return a single material ID at this position in space.");

  // we screen to be sure there's just one material fill, so "first" here is same as "only"
  auto first_material_cell = firstContainedMaterialCell(cell_info);

  int32_t index;
  _openmc_problem->materialFill(first_material_cell, &index);

  return _openmc_problem->materialID(index);;
}

#endif
