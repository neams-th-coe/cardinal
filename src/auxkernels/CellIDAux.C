#include "CellIDAux.h"

registerMooseObject("OpenMCApp", CellIDAux);

template<>
InputParameters validParams<CellIDAux>()
{
  InputParameters params = validParams<OpenMCAuxKernel>();
  params.addClassDescription("Display the OpenMC cell ID mapped to each MOOSE element");
  return params;
}

CellIDAux::CellIDAux(const InputParameters & parameters) :
    OpenMCAuxKernel(parameters)
{
}

Real
CellIDAux::computeValue()
{
  return _openmc_problem->elemToCellID(_current_elem->id());
}
