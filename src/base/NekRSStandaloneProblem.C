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
    mooseWarning("This class does not transfer mesh displacements from NekRS to Cardinal.\n"
                 "The [Mesh] in Cardinal won't reflect NekRS's internal mesh changes.");

  _minimum_scratch_size_for_coupling = 0;
  if (!params.isParamSetByUser("n_usrwrk_slots"))
    _n_usrwrk_slots = 0;
}
#endif
