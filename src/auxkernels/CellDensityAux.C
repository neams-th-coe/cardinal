#include "CellDensityAux.h"
#include "CardinalEnums.h"

registerMooseObject("CardinalApp", CellDensityAux);

template<>
InputParameters validParams<CellDensityAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC fluid density (kg/m3) mapped to each MOOSE element");
  return params;
}

CellDensityAux::CellDensityAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
{
}

Real
CellDensityAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a density of -1; otherwise, we would
  // get an error in the call to cellCouplingFields, since it relies on the
  // OpenMCCellAverageProblem::_cell_to_elem std::map that wouldn't have an entry that corresponds
  // to an unmapped cell
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());

  // we only extract the material information for fluid cells, because otherwise we don't
  // need to know the material info. So, set a value of -1 for non-fluid cells.
  if (_openmc_problem->cellCouplingFields(cell_info) != coupling::density_and_temperature)
    return OpenMCCellAverageProblem::UNMAPPED;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);

  double density;
  int err = openmc_material_get_density(index, &density);

  if (err)
    mooseError("In attempting to get density for " + _openmc_problem->printMaterial(index)
      + ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return density / _openmc_problem->densityConversionFactor();
}
