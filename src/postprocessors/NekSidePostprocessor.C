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

#include "NekSidePostprocessor.h"

InputParameters
NekSidePostprocessor::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params.addRequiredParam<std::vector<int>>(
      "boundary", "Boundary ID(s) for which to compute the postprocessor");
  return params;
}

NekSidePostprocessor::NekSidePostprocessor(const InputParameters & parameters)
  : NekPostprocessor(parameters), _boundary(getParam<std::vector<int>>("boundary"))
{
  const auto & filename = getMooseApp().getInputFileName();

  // check that each specified boundary is within the range [1, n_fluid_boundaries]
  // that nekRS recognizes for its problem
  int first_invalid_id, n_boundaries;
  bool valid_ids = nekrs::validBoundaryIDs(_boundary, first_invalid_id, n_boundaries);

  if (!valid_ids)
    mooseError("Invalid 'boundary' entry: ",
               first_invalid_id,
               "\n\n"
               "nekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
               "For this problem, nekRS has ",
               n_boundaries,
               " boundaries. "
               "Did you enter a valid 'boundary' in '" +
                   filename + "'?");
}

#endif
