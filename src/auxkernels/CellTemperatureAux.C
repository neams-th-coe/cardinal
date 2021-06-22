#include "CellTemperatureAux.h"
#include "openmc/cell.h"
#include "openmc/error.h"

registerMooseObject("CardinalApp", CellTemperatureAux);

template<>
InputParameters validParams<CellTemperatureAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC cell temperature (K) at each MOOSE element");
  return params;
}

CellTemperatureAux::CellTemperatureAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
{
}

Real
CellTemperatureAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a temperature of -1; this is required
  // because otherwise OpenMC would throw an error for an invalid instance, index pair passed to the C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());

  return _openmc_problem->cellTemperature(cell_info);
}
