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

#ifdef ENABLE_NEK_COUPLING

#include "NekRSStandaloneProblem.h"
#include "NekInterface.h"
#include "UserErrorChecking.h"

registerMooseObject("CardinalApp", NekRSStandaloneProblem);

InputParameters
NekRSStandaloneProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
  return params;
}

NekRSStandaloneProblem::NekRSStandaloneProblem(const InputParameters & params)
  : NekRSProblemBase(params)
{
  // It doesn't make sense to specify both a boundary and volume for a
  // standalone case, because we are only using the boundary/volume for
  // data extraction (and if you set 'volume = true', you automatically get
  // the solution on the boundaries as well).
  if (_nek_mesh->boundary() && _nek_mesh->volume())
    mooseWarning("When 'volume = true' for '" + type() +
                 "', it is redundant to also set 'boundary'.\n"
                 "Boundary IDs will be ignored.");

  if (nekrs::hasMovingMesh())
    mooseWarning("NekRSStandaloneProblem currently does not transfer mesh displacements "
                 "from NekRS to Cardinal. The [Mesh] object in Cardinal won't reflect "
                 "nekRS's internal mesh changes. This may affect your postprocessor values.");

  // Cardinal does not automatically allocate any scratch space for this class
  if (params.isParamSetByUser("n_usrwrk_slots"))
    checkUnusedParam(params, "n_usrwrk_slots", "using running NekRS as a standalone "
      "problem through Cardinal");
  _n_usrwrk_slots = 0;
}
#endif
