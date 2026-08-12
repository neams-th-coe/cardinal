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

registerMooseObject("CardinalApp", CellDensityAux);

InputParameters
CellDensityAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("OpenMC cell density (kg/m$^3$), mapped to each MOOSE element. For "
                             "cells filled by universes or lattices, returns the density of the "
                             "first-located, non-void, material cell contained within.");
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

  if (!_openmc_problem->hasDensityFeedback(cell_info))
    return OpenMCCellAverageProblem::UNMAPPED;

  return _openmc_problem->cellDensity(cell_info, _current_elem);
}

#endif
