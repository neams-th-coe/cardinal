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

#include "CellInstanceAux.h"

registerMooseObject("CardinalApp", CellInstanceAux);

InputParameters
CellInstanceAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("OpenMC cell instance, mapped to each MOOSE element");
  return params;
}

CellInstanceAux::CellInstanceAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    OpenMCBase(this, parameters)
{
}

Real
CellInstanceAux::computeValue()
{
  // no special catch for unmapped elements is required here, because elemToCellInstance
  // returns -1 for UNMAPPED elements
  return _openmc_problem->elemToCellInstance(_current_elem->id());
}

#endif
