#include "FissionTallyRelativeError.h"
#include "MooseUtils.h"
#include "xtensor/xview.hpp"

registerMooseObject("CardinalApp", FissionTallyRelativeError);

InputParameters
FissionTallyRelativeError::validParams()
{
  InputParameters params = OpenMCPostprocessor::validParams();
  params.addParam<MooseEnum>("value_type", getOperationEnum(),
    "Whether to give the maximum or minimum tally relative error; options: 'max' (default), 'min'");
  params.addClassDescription("Extract the maximum/minimum fission tally relative error");
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

    for (int i = 0; i < t->n_filter_bins(); ++i)
    {
      // tallies without any scores to them will have zero error, which doesn't really make
      // sense to compare against
      if (MooseUtils::absoluteFuzzyEqual(sum(i), 0))
        continue;

      Real rel_err = _openmc_problem->relativeError(sum(i), sum_sq(i), t->n_realizations_);

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
