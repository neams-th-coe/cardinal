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

#include "ElementOpticalDepthIndicator.h"

#include "CardinalEnums.h"
#include "TallyBase.h"

registerMooseObject("CardinalApp", ElementOpticalDepthIndicator);

InputParameters
ElementOpticalDepthIndicator::validParams()
{
  auto params = OpenMCIndicator::validParams();
  params.addClassDescription(
      "A class which returns the estimate of a given element's optical depth under the assumption "
      "that "
      "elements with a large optical depth experience large solution gradients.");
  params.addRequiredParam<MooseEnum>("rxn_rate",
                                     getSingleTallyScoreEnum(),
                                     "The reaction rate to use for computing the optical depth.");
  params.addParam<MooseEnum>(
      "h_type",
      MooseEnum("min max cube_root", "max"),
      "The estimate for the length of the element used to compute the optical depth. Options are "
      "the "
      "minimum vertex separation (min), the maximum vertex separation (max), and the cube root of "
      "the element volume (cube_root).");
  params.addParam<bool>("invert", false, "Whether the optical depth is stored as 1 / OD or OD.");

  return params;
}

ElementOpticalDepthIndicator::ElementOpticalDepthIndicator(const InputParameters & parameters)
  : OpenMCIndicator(parameters), _h_type(getParam<MooseEnum>("h_type").getEnum<HType>()), _invert(getParam<bool>("invert"))
{
  std::string score = getParam<MooseEnum>("rxn_rate");
  std::replace(score.begin(), score.end(), '_', '-');

  // Error check to make sure the score is a reaction rate score and to make sure one of the
  // [Tallies] has added the score and a flux score.
  if (!_openmc_problem->isReactionRateScore(score))
    paramError(
        "rxn_rate",
        "At present the ElementOpticalDepthIndicator only works with reaction rate scores. " +
            std::string(getParam<MooseEnum>("rxn_rate")) + " is not a valid reaction rate score.");

  if (!_openmc_problem->hasScore(score))
    paramError("rxn_rate",
               "The problem does not contain any score named " +
                   std::string(getParam<MooseEnum>("rxn_rate")) +
                   "! Please "
                   "ensure that one of your [Tallies] is scoring the requested reaction rate.");

  if (!_openmc_problem->hasScore("flux"))
    mooseError("In order to use an ElementOpticalDepthIndicator one of your [Tallies] must add a "
               "flux score.");

  // Check to ensure the reaction rate / flux variables are CONSTANT MONOMIALS.
  bool const_mon = true;
  for (const auto v : _openmc_problem->getTallyScoreVariables(score, _tid, "", true))
    const_mon &= v->feType() == FEType(CONSTANT, MONOMIAL);
  for (const auto v : _openmc_problem->getTallyScoreVariables("flux", _tid, "", true))
    const_mon &= v->feType() == FEType(CONSTANT, MONOMIAL);

  if (!const_mon)
    paramError("rxn_rate",
               "ElementOpticalDepthIndicator only supports CONSTANT MONOMIAL field variables. "
               "Please ensure your [Tallies] are adding CONSTANT MONOMIAL field variables.");

  // Grab the reaction rate / flux variables from the [Tallies].
  _rxn_rates = _openmc_problem->getTallyScoreVariableValues(score, _tid, "", true);
  _scalar_fluxes = _openmc_problem->getTallyScoreVariableValues("flux", _tid, "", true);
}

void
ElementOpticalDepthIndicator::computeIndicator()
{
  Real rxn_rate = 0.0;
  Real scalar_flux = 0.0;

  for (const auto & var : _rxn_rates)
    rxn_rate += (*var)[0];

  for (const auto & var : _scalar_fluxes)
    scalar_flux += (*var)[0];

  auto od = scalar_flux < (libMesh::TOLERANCE * libMesh::TOLERANCE) ? 0.0 : rxn_rate / scalar_flux;

  switch (_h_type)
  {
    case HType::Min:
      od /= _current_elem->hmin();
      break;
    case HType::Max:
      od /= _current_elem->hmax();
      break;
    case HType::CubeRoot:
      od /= std::cbrt(_current_elem->volume());
      break;
    default:
      mooseError("Internal error: unhandled HType enum state in ElementOpticalDepthIndicator.");
      break;
  }

  if (_invert && scalar_flux > libMesh::TOLERANCE * libMesh::TOLERANCE)
    _field_var.setNodalValue(1.0 / od);
  else
  _field_var.setNodalValue(od);
}

#endif
