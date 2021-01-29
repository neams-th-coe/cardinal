//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NekSidePostprocessor.h"

defineLegacyParams(NekSidePostprocessor);

InputParameters
NekSidePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<std::vector<int>>("boundary", "Boundary ID(s) for which to compute the postprocessor");
  return params;
}

NekSidePostprocessor::NekSidePostprocessor(const InputParameters & parameters) :
  GeneralPostprocessor(parameters),
  _boundary(getParam<std::vector<int>>("boundary"))
{
  const auto & filename = getMooseApp().getInputFileName();

  // check that each specified boundary is within the range [1, n_fluid_boundaries]
  // that nekRS recognizes for its problem
  int first_invalid_id, n_boundaries;
  bool valid_ids = nekrs::mesh::validBoundaryIDs(_boundary, first_invalid_id, n_boundaries);

  if (!valid_ids)
    mooseError("Invalid 'boundary' entry specified for 'NekSidePostprocessor': ", first_invalid_id, "\n\n"
      "nekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
      "For this problem, nekRS has ", n_boundaries, " boundaries. "
      "Did you enter a valid 'boundary' in '" + filename + "'?");
}
