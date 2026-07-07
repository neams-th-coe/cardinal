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

#include "ModelModifiersBase.h"
#include "UserErrorChecking.h"
#include "VariadicTable.h"
#include "BrentsMethod.h"

// To disable tallies
#include "openmc/tallies/tally.h"

InputParameters
ModelModifiersBase::validParams()
{
  auto params = MooseObject::validParams();
  params += OpenMCBase::validParams();

  params.addClassDescription(
      "Base class for classes that modify the OpenMC model prior to execution");
  params.registerBase("ModelModifiers");
  params.registerSystemAttributeName("ModelModifiers");
  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");
  return params;
}

ModelModifiersBase::ModelModifiersBase(const InputParameters & parameters)
  : MooseObject(parameters),
    OpenMCBase(this, parameters)
{
}

#endif
