#include "CellDensityAux.h"
#include "CardinalEnums.h"

registerMooseObject("OpenMCApp", CellDensityAux);

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
  // we only extract the material information for fluid cells, because otherwise we don't
  // need to know the material info. So, set a value of -1 for non-fluid cells.
  std::pair<int32_t, int32_t> cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());
  if (_openmc_problem->cellPhase(cell_info) != coupling::density_and_temperature)
    return -1;

  int32_t index = _openmc_problem->cellToMaterialIndex(cell_info);

  double density;
  int err = openmc_material_get_density(index, &density);

  if (err)
    mooseError("In attempting to get density for " + _openmc_problem->printMaterial(index)
      + ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return density / _openmc_problem->densityConversionFactor();
}
