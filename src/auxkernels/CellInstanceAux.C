#include "CellInstanceAux.h"

registerMooseObject("OpenMCApp", CellInstanceAux);

template<>
InputParameters validParams<CellInstanceAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC cell Instance mapped to each MOOSE element");
  return params;
}

CellInstanceAux::CellInstanceAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
{
}

Real
CellInstanceAux::computeValue()
{
  return _openmc_problem->elemToCellInstance(_current_elem->id());
}
