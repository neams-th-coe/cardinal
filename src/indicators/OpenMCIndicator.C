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

#include "OpenMCIndicator.h"

#include "libmesh/threads.h"

InputParameters
OpenMCIndicator::validParams()
{
  auto params = Indicator::validParams();
  return params;
}

OpenMCIndicator::OpenMCIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    TallyInterface(this, parameters),
    _field_var(_subproblem.getStandardVariable(_tid, name())),
    _current_elem(_field_var.currentElem())
{
}

#endif
