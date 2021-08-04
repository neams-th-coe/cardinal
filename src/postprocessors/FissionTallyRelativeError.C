//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FissionTallyRelativeError.h"
#include "xtensor/xview.hpp"

registerMooseObject("OpenMCApp", FissionTallyRelativeError);

defineLegacyParams(FissionTallyRelativeError);

InputParameters
FissionTallyRelativeError::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type", getOperationEnum(), "Operation to perform to get extreme value");
  return params;
}

FissionTallyRelativeError::FissionTallyRelativeError(const InputParameters & parameters) :
  OpenMCPostprocessor(parameters),
  _type(getParam<MooseEnum>("value_type").getEnum<operation::OperationEnum>())
{
}

Real
FissionTallyRelativeError::getValue()
{
  const auto & tally = _openmc_problem->getLocalTally();

  Real extreme_value;

  switch (_type)
  {
    case operation::max:
      extreme_value = std::numeric_limits<Real>::min();
      break;
    case operation::min:
      extreme_value = std::numeric_limits<Real>::max();
      break;
    default:
      mooseError("Unhandled OperationEnum!");
  }

  for (const auto & t : tally)
  {
    auto sum = xt::view(t->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
    auto sum_sq = xt::view(t->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM_SQ));
    int n = t->n_realizations_;

    for (int i = 0; i < t->n_filter_bins(); ++i)
    {
      auto mean = sum(i) / n;
      Real std_dev = std::sqrt((sum_sq(i) / n - mean * mean) / (n - 1));
      Real rel_err = mean != 0.0 ? std_dev / std::abs(mean) : 0.0;

      switch (_type)
      {
        case operation::max:
          extreme_value = std::max(extreme_value, rel_err);
          break;
        case operation::min:
          extreme_value = std::min(extreme_value, rel_err);
          break;
        default:
          mooseError("Unhandled OperationEnum!");
      }
    }
  }

  return extreme_value;
}
