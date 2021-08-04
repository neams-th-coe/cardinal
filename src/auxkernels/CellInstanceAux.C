#include "CellInstanceAux.h"

registerMooseObject("CardinalApp", CellInstanceAux);

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
  // no special catch for unmapped elements is required here, because elemToCellInstance
  // returns -1 for UNMAPPED elements
  return _openmc_problem->elemToCellInstance(_current_elem->id());
}
