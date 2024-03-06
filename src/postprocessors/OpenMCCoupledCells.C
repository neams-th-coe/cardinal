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

#include "OpenMCCoupledCells.h"

registerMooseObject("CardinalApp", OpenMCCoupledCells);

InputParameters
OpenMCCoupledCells::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addClassDescription("Number of OpenMC cells receiving temperature/density feedback or "
                             "sending a cell tally to MOOSE");
  return params;
}

OpenMCCoupledCells::OpenMCCoupledCells(const InputParameters & parameters)
  : OpenMCPostprocessor(parameters)
{
}

Real
OpenMCCoupledCells::getValue() const
{
  return _openmc_problem->cellToElem().size();
}

#endif
