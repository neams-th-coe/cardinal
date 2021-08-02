#include "CellMaterialIDAux.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", CellMaterialIDAux);

template<>
InputParameters validParams<CellMaterialIDAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC fluid material ID mapped to each MOOSE element");
  return params;
}

CellMaterialIDAux::CellMaterialIDAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
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
  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());
  if (_openmc_problem->cellCouplingFields(cell_info) != coupling::density_and_temperature)
    return -1;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);
  int32_t id = _openmc_problem->materialID(index);

  return id;
}
