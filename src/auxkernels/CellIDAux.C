/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_OPENMC_COUPLING

#include "CellIDAux.h"

registerMooseObject("CardinalApp", CellIDAux);

InputParameters
CellIDAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("Display the OpenMC cell ID mapped to each MOOSE element");
  return params;
}

CellIDAux::CellIDAux(const InputParameters & parameters) : OpenMCAuxKernel(parameters) {}

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

#endif
