#include "CellTemperatureAux.h"
#include "openmc/cell.h"
#include "openmc/error.h"

registerMooseObject("OpenMCApp", CellTemperatureAux);

template<>
InputParameters validParams<CellTemperatureAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC cell temperature at each MOOSE element");
  return params;
}

CellTemperatureAux::CellTemperatureAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
{
}

Real
CellTemperatureAux::computeValue()
{
  std::pair<int32_t, int32_t> cell_info = _openmc_problem->elemToCellInfo(_current_elem->id());

  double T;
  int err = openmc_cell_get_temperature(cell_info.first, &cell_info.second, &T);

  if (err)
    mooseError("In attempting to get temperature of " + _openmc_problem->printCell(cell_info) +
      ", OpenMC reported:\n\n" + std::string(openmc_err_msg));

  return T;
}
