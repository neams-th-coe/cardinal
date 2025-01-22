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
  InputParameters params = AuxKernel::validParams();
  params += OpenMCBase::validParams();

  MooseEnum vol_type("mapped actual");
  params.addRequiredParam<MooseEnum>("volume_type", vol_type,
    "Which notion of cell volume to display. For 'mapped', this shows only the volume of the "
    "MOOSE elements which map to each cell (this is mostly for testing). For 'actual', this "
    "will map to the [Mesh] the actual volumes of OpenMC cells obtained from a stochastic "
    "calculation");
  params.addClassDescription("OpenMC cell volumes, mapped to MOOSE");
  return params;
}

CellVolumeAux::CellVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    OpenMCBase(this, parameters),
    _volume_type(getParam<MooseEnum>("volume_type"))
{
}

Real
CellVolumeAux::computeValue()
{
  if (_volume_type == "actual" && !_openmc_problem->volumeCalculation())
    paramError("volume_type",
               "To display the actual OpenMC cell volumes, the [Problem] block needs to set the\n"
               "'volume_calculation' parameter.");

  // if the element doesn't map to an OpenMC cell, return a volume of -1; this is required
  // because otherwise OpenMC would throw an error for an invalid instance, index pair passed to the
  // C-API
  if (!mappedElement(_current_elem))
    return OpenMCCellAverageProblem::UNMAPPED;

  OpenMCCellAverageProblem::cellInfo cell_info =
      _openmc_problem->elemToCellInfo(_current_elem->id());

  if (_volume_type == "mapped")
    return _openmc_problem->cellMappedVolume(cell_info);
  else if (_volume_type == "actual")
    return _openmc_problem->cellVolume(cell_info);
  else
    mooseError("Unhandled vol_type enum in CellVolumeAux!");
}

#endif
