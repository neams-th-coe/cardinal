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

registerMooseObject("CardinalApp", NekRSStandaloneProblem);

InputParameters
NekRSStandaloneProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
  return params;
}

NekRSStandaloneProblem::NekRSStandaloneProblem(const InputParameters & params)
  : NekRSProblemBase(params),
    _usrwrk_indices(MultiMooseEnum("unused"))
{
  // It doesn't make sense to specify both a boundary and volume for a
  // standalone case, because we are only using the boundary/volume for
  // data extraction (and if you set 'volume = true', you automatically get
  // the solution on the boundaries as well).
  if (_nek_mesh->boundary() && _nek_mesh->volume())
    mooseWarning("When 'volume = true' for '" + type() +
                 "', it is redundant to also set 'boundary'.\n"
                 "Boundary IDs will be ignored.");

  // Cardinal does not write any data to nsr->usrwrk for this class, so all
  // slices are unused
  std::vector<std::string> indices;
  _minimum_scratch_size_for_coupling = 0;
  for (unsigned int i = _minimum_scratch_size_for_coupling; i < _n_usrwrk_slots; ++i)
    indices.push_back("unused");

  _usrwrk_indices = indices;

  printScratchSpaceInfo(_usrwrk_indices);
}

bool
NekRSStandaloneProblem::movingMesh() const
{
  return nekrs::hasMovingMesh();
}

#endif
