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

#include "CellVolumeAux.h"
#include "openmc/cell.h"
#include "openmc/error.h"

registerMooseObject("CardinalApp", CellVolumeAux);

InputParameters
CellVolumeAux::validParams()
{
  InputParameters params = OpenMCAuxKernel::validParams();
  params.addClassDescription("Display the OpenMC cell mapped volumes");
  return params;
}

CellVolumeAux::CellVolumeAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters)
{
}

Real
CellVolumeAux::computeValue()
{
  // if the element doesn't map to an OpenMC cell, return a volume of -1; this is required
  // because otherwise OpenMC would throw an error for an invalid instance, index pair passed to the
  // C-API
  if (!mappedElement())
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());

  return _openmc_problem->cellMappedVolume(cell_info);
}

#endif
