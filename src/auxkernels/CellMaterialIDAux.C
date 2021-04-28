#include "CellMaterialIDAux.h"
#include "CardinalEnums.h"

registerMooseObject("OpenMCApp", CellMaterialIDAux);

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
  // we only extract the material information for fluid cells, because otherwise we don't
  // need to know the material info. So, set a value of -1 for non-fluid cells.
  std::pair<int32_t, int32_t> cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());
  if (_openmc_problem->cellPhase(cell_info) != coupling::density_and_temperature)
    return -1;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);
  int32_t id = _openmc_problem->materialID(index);

  return id;
}
