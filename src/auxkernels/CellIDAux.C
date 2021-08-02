#include "CellIDAux.h"

registerMooseObject("CardinalApp", CellIDAux);

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
  // if the element doesn't map to an OpenMC cell, return a cell ID of -1; otherwise, we would
  // get an error in the call to cellID, since it relies on a valid cell instance, index pair being
  // passed to OpenMC's C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  return _openmc_problem->elemToCellID(_current_elem->id());
}
