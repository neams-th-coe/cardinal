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

#include "SpatialLegendreAux.h"

#include "CardinalEnums.h"
#include "SpatialLegendreFilter.h"
#include "TallyBase.h"

#include "openmc/math_functions.h"

registerMooseObject("CardinalApp", SpatialLegendreAux);

InputParameters
SpatialLegendreAux::validParams()
{
  auto params = OpenMCAuxKernel::validParams();
  params.addClassDescription(
      "An auxkernel which evaluates a tally that has been expanded in Legendre polynomials of "
      "position by a SpatialLegendreFilter.");
  params.addRequiredParam<MooseEnum>("score",
                                     getSingleTallyScoreEnum(),
                                     "The tally score this auxkernel should evaluate the Legendre "
                                     "expansion of.");
  params.addParam<std::string>(
      "tally",
      "The name of the tally to fetch the score variables from. Only required if "
      "your problem contains multiple tallies which accumulate the same score.");
  params.addParam<std::string>("filter",
                               "The name of the SpatialLegendreFilter to evaluate. Only required "
                               "if the tally has more than one SpatialLegendreFilter applied.");
  params.addParam<unsigned int>(
      "ext_filter_bin",
      0,
      "The flattened bin index for any other filters applied to this tally with [Filters] (bin "
      "indices start at 0). This parameter should be specified if you wish to evaluate the "
      "Legendre expansion in a different non-spatial tally bin, such as a different energy group.");
  MooseEnum normalization("volumetric raw none", "volumetric");
  params.addParam<MooseEnum>(
      "normalization",
      normalization,
      "How to weight each expansion coefficient when summing the Legendre series.\n"
      "  'volumetric': multiply moment l by (2l + 1). Use this for the tally variables "
      "Cardinal writes, which have already been divided by the mapped volume of the tally "
      "bin. That volume carries the length of the expansion domain, so the 1/(maximum - "
      "minimum) factor of the Legendre series is already applied and must not be applied "
      "again. This is the correct choice for reconstructing a volumetric tally.\n"
      "  'raw': multiply moment l by (2l + 1) / (maximum - minimum). Use this if the "
      "coefficients are raw expansion integrals that have NOT been divided by a volume, "
      "matching OpenMC's openmc.legendre_from_expcoef utility.\n"
      "  'none': sum the polynomials against the coefficients with no extra factors.");
  params.addParam<bool>("normalize_coefficients",
                        "Deprecated; use 'normalization' instead. 'true' maps to "
                        "'normalization = raw' and 'false' to 'normalization = none'.");

  return params;
}

SpatialLegendreAux::SpatialLegendreAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters),
    _legendre_filter(nullptr),
    _normalization(getParam<MooseEnum>("normalization").getEnum<Normalization>())
{
  if (isParamSetByUser("normalize_coefficients"))
  {
    if (isParamSetByUser("normalization"))
      paramError("normalize_coefficients",
                 "'normalize_coefficients' is deprecated and cannot be combined with "
                 "'normalization'. Please set only 'normalization'.");

    _normalization =
        getParam<bool>("normalize_coefficients") ? Normalization::RAW : Normalization::NONE;
    mooseDeprecated("'normalize_coefficients' is deprecated; use 'normalization = ",
                    getParam<bool>("normalize_coefficients") ? "raw" : "none",
                    "' instead. Note that for reconstructing the volumetric tally variables "
                    "Cardinal writes, the correct setting is 'normalization = volumetric'.");
  }

  /**
   * Note that unlike most of Cardinal's tally auxkernels, 'variable' is not restricted to
   * CONSTANT MONOMIAL. The expansion is a continuous function of position, so it can be
   * projected onto an elemental variable of any order by evaluating it at each quadrature
   * point. Nodal variables are rejected by OpenMCAuxKernel, since the tally coefficient
   * variables this object reads are elemental.
   */
  const auto score = getScore("score");
  const auto tally_name = tallyByScore(score, "tally");
  const auto * tally = _openmc_problem->getTally(tally_name);

  // Find the SpatialLegendreFilter applied to this tally, and count the number of bins
  // in the filters which are nested inside of it (the filters which were added after it).
  const auto & ext_filters = tally->getExtFilters();
  unsigned int inner_bins = 1;
  unsigned int outer_bins = 1;
  unsigned int num_legendre_filters = 0;
  for (const auto & f : ext_filters)
  {
    const auto * legendre = dynamic_cast<const SpatialLegendreFilter *>(f.get());

    // Only count filters which aren't the one we're evaluating.
    const bool is_match =
        legendre && (!isParamValid("filter") || f->name() == getParam<std::string>("filter"));

    if (is_match)
    {
      _legendre_filter = legendre;
      num_legendre_filters++;

      // Any bins accumulated so far belong to filters outside of this one.
      outer_bins = inner_bins;
      inner_bins = 1;
    }
    else
      inner_bins *= f->numBins();
  }

  if (!_legendre_filter)
  {
    if (isParamValid("filter"))
      paramError("filter",
                 "The tally '" + tally_name + "' does not have a SpatialLegendreFilter named '" +
                     getParam<std::string>("filter") + "' applied to it!");
    else
      paramError("score",
                 "The tally '" + tally_name + "' accumulating the score '" + score +
                     "' does not have a SpatialLegendreFilter applied to it! Please add one in the "
                     "[Problem/Filters] block and apply it to the tally with 'filters'.");
  }

  if (num_legendre_filters > 1)
    paramError("filter",
               "The tally '" + tally_name +
                   "' has multiple SpatialLegendreFilter(s) applied to it. Please specify which "
                   "one this auxkernel should evaluate with 'filter'.");

  _order = _legendre_filter->getOrder();
  _axis = _legendre_filter->getAxis();
  _polynomials.resize(_order + 1);

  // Grab the tally variables; these are flattened over all of the external filter bins, with
  // the bins of the last-added filter varying fastest.
  auto score_vars = _openmc_problem->getTallyScoreVariables(score, tally_name, _tid);
  auto score_bins = _openmc_problem->getTallyScoreVariableValues(score, tally_name, _tid);

  const unsigned int legendre_bins = _legendre_filter->numBins();
  const unsigned int other_bins = outer_bins * inner_bins;
  const auto bin_index = getParam<unsigned int>("ext_filter_bin");
  if (bin_index >= other_bins)
    paramError("ext_filter_bin",
               "The external filter bin provided is invalid for the number of external filter "
               "bins (" +
                   Moose::stringify(other_bins) + ") applied to '" + score +
                   "' which do not belong to the SpatialLegendreFilter!");

  // Decompose the requested bin index into the filters outside / inside of the Legendre filter,
  // and then re-flatten with the Legendre bin inserted in the middle.
  const unsigned int outer_index = bin_index / inner_bins;
  const unsigned int inner_index = bin_index % inner_bins;
  for (unsigned int l = 0; l < legendre_bins; ++l)
  {
    const unsigned int flat_index =
        outer_index * legendre_bins * inner_bins + l * inner_bins + inner_index;

    const auto fe_type = score_vars[flat_index]->feType();
    if (fe_type.family != libMesh::MONOMIAL || fe_type.order != libMesh::CONSTANT)
      paramError("score",
                 "SpatialLegendreAux only supports CONSTANT MONOMIAL shape functions for tally "
                 "variables.");

    _coefficients.push_back(score_bins[flat_index]);
  }
}

Real
SpatialLegendreAux::computeValue()
{
  /**
   * Evaluate at the current quadrature point so that the (continuous) expansion can be
   * projected onto higher-order variables. For CONSTANT MONOMIAL variables there is a single
   * quadrature point at the element centroid, which recovers a centroid evaluation.
   */
  const Point pt = _openmc_problem->transformPoint(_q_point[_qp]) * _openmc_problem->scaling();
  const Real x = pt(_axis);

  /**
   * Read the limits from the filter on every evaluation rather than caching them at
   * construction. A filter using 'track_mesh' re-derives its limits from the [Mesh] each
   * time the tallies are re-initialized, so a cached copy goes stale as soon as the domain
   * deforms. That previously made the reconstruction return zero in the outermost elements
   * (which had expanded past the initial limits), silently losing part of the score.
   */
  const Real min = _legendre_filter->getMinimum();
  const Real max = _legendre_filter->getMaximum();

  // The filter doesn't score anything outside of the expansion limits.
  if (x < min || x > max)
    return 0.0;

  const Real x_norm = 2.0 * (x - min) / (max - min) - 1.0;
  openmc::calc_pn_c(_order, x_norm, _polynomials.data());

  Real value = 0.0;
  for (unsigned int l = 0; l <= _order; ++l)
  {
    Real factor = 1.0;
    switch (_normalization)
    {
      case Normalization::VOLUMETRIC:
        factor = 2.0 * l + 1.0;
        break;
      case Normalization::RAW:
        factor = (2.0 * l + 1.0) / (max - min);
        break;
      case Normalization::NONE:
        factor = 1.0;
        break;
    }
    value += factor * (*_coefficients[l])[_qp] * _polynomials[l];
  }

  return value;
}

#endif
