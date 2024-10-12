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
#include "FilterBase.h"

#include "OpenMCCellAverageProblem.h"

InputParameters
FilterBase::validParams()
{
  auto params = MooseObject::validParams();
  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");

  params.registerBase("Filter");
  params.registerSystemAttributeName("Filter");

  return params;
}

FilterBase::FilterBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _openmc_problem(*getParam<OpenMCCellAverageProblem *>("_openmc_problem"))
{
}

openmc::Filter *
FilterBase::getWrappedFilter()
{
  // Error check the API to make sure the filter has been initialized.
  if (!_filter)
    mooseError(_name + " has not initialized an OpenMC filter!");

  return _filter;
}
#endif
