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

#include "CellTemperatureAux.h"
#include "openmc/cell.h"
#include "openmc/error.h"

registerMooseObject("CardinalApp", CellTemperatureAux);

InputParameters
CellTemperatureAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("Display the OpenMC cell temperature (K) at each MOOSE element");
  return params;
}

CellTemperatureAux::CellTemperatureAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters)
{
}

Real
CellTemperatureAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a temperature of -1; this is required
  // because otherwise OpenMC would throw an error for an invalid instance, index pair passed to the
  // C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());

  if (!_openmc_problem->hasTemperatureFeedback(cell_info))
    return OpenMCCellAverageProblem::UNMAPPED;

  return _openmc_problem->cellTemperature(cell_info);
}

#endif
