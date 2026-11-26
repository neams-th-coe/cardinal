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

#include "SpatialLegendreFilter.h"

#include "OpenMCCellAverageProblem.h"

registerMooseObject("CardinalApp", SpatialLegendreFilter);

InputParameters
SpatialLegendreFilter::validParams()
{
  auto params = FilterBase::validParams();
  params.addClassDescription(
      "A class which provides a thin wrapper around an OpenMC SpatialLegendreFilter. This filter "
      "expands the tally score in Legendre polynomials of the particle position along one of the "
      "Cartesian axes. The expansion coefficients can be evaluated on the [Mesh] with a "
      "SpatialLegendreAux.");
  params.addRequiredParam<unsigned int>("order", "The order of the Legendre expansion.");
  params.addRequiredParam<MooseEnum>(
      "axis", MooseEnum("x y z"), "The Cartesian axis the Legendre expansion is applied along.");
  params.addParam<Real>("minimum",
                        "The minimum coordinate along 'axis' which the expansion covers, "
                        "in units of the [Mesh]. Not used if 'use_displaced_mesh' is set.");
  params.addParam<Real>("maximum",
                        "The maximum coordinate along 'axis' which the expansion covers, "
                        "in units of the [Mesh]. Not used if 'use_displaced_mesh' is set.");
  params.addParam<bool>(
      "use_displaced_mesh",
      false,
      "Whether to take the expansion limits from the extent of the [Mesh] along 'axis', "
      "refreshing them each time the tallies are re-initialized. Set this when the domain "
      "deforms during the solve (e.g. thermal expansion with a MoabSkinner) so that the "
      "expansion continues to span the domain instead of remaining fixed at its initial "
      "extent. When set, 'minimum' and 'maximum' must not be provided.");
  params.addParam<std::vector<SubdomainName>>(
      "block",
      "Restrict the extent used by 'use_displaced_mesh' to these subdomains. Use this to build an "
      "element-local expansion: give each tally its own filter restricted to the same block, "
      "so that the Legendre basis spans just that element rather than the whole domain. An "
      "expansion whose basis spans the whole domain cannot represent a score that is "
      "localized to one cell.");

  return params;
}

SpatialLegendreFilter::SpatialLegendreFilter(const InputParameters & parameters)
  : FilterBase(parameters),
    _order(getParam<unsigned int>("order")),
    _axis(getParam<MooseEnum>("axis")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _min(0.0),
    _max(0.0)
{
  if (_openmc_problem.runRandomRay())
    mooseError(
        "SpatialLegendreFilter is presently not supported when running the random ray solver!");

  if (_use_displaced_mesh)
  {
    if (isParamValid("minimum") || isParamValid("maximum"))
      paramError("use_displaced_mesh",
                 "'minimum' and 'maximum' cannot be specified when 'use_displaced_mesh' is set, "
                 "because the expansion limits are taken from the [Mesh] extent.");

    std::tie(_min, _max) = limitsFromMesh();
  }
  else
  {
    if (!isParamValid("minimum") || !isParamValid("maximum"))
      paramError("minimum",
                 "'minimum' and 'maximum' are both required unless 'use_displaced_mesh' is set.");

    // The limits are given in [Mesh] units, but OpenMC's filter works in centimeters.
    _min = getParam<Real>("minimum") * _openmc_problem.scaling();
    _max = getParam<Real>("maximum") * _openmc_problem.scaling();
  }

  if (_max <= _min)
    paramError("maximum", "'maximum' must be greater than 'minimum'!");

  auto legendre_filter =
      dynamic_cast<openmc::SpatialLegendreFilter *>(openmc::Filter::create("spatiallegendre"));

  try
  {
    legendre_filter->set_order(_order);
  }
  catch (const std::exception & e)
  {
    paramError("order", e.what());
  }

  legendre_filter->set_axis(static_cast<openmc::LegendreAxis>(_axis));
  legendre_filter->set_minmax(_min, _max);

  _filter = legendre_filter;
}

std::pair<Real, Real>
SpatialLegendreFilter::limitsFromMesh() const
{
  // The filter limits live in OpenMC's coordinate system, so the [Mesh] extent must be
  // transformed and scaled the same way element centroids are when they are mapped.
  auto & moose_mesh = _openmc_problem.getMooseMesh();
  const auto & mesh = moose_mesh.getMesh();

  // When restricted to a set of blocks, only those elements contribute to the extent;
  // this is how an element-local expansion is built.
  std::set<SubdomainID> blocks;
  if (isParamValid("block"))
  {
    const auto ids = moose_mesh.getSubdomainIDs(getParam<std::vector<SubdomainName>>("block"));
    blocks.insert(ids.begin(), ids.end());
  }

  Real min = std::numeric_limits<Real>::max();
  Real max = std::numeric_limits<Real>::lowest();

  auto accumulate = [&](const Point & p)
  {
    const Real x = (_openmc_problem.transformPoint(p) * _openmc_problem.scaling())(_axis);
    min = std::min(min, x);
    max = std::max(max, x);
  };

  if (blocks.empty())
    for (const auto * node : mesh.node_ptr_range())
      accumulate(*node);
  else
    for (const auto * elem : mesh.active_element_ptr_range())
      if (blocks.count(elem->subdomain_id()))
        for (const auto & n : elem->node_ref_range())
          accumulate(n);

  // The mesh may be distributed, so the extent must be reduced over all ranks.
  _openmc_problem.comm().min(min);
  _openmc_problem.comm().max(max);

  return {min, max};
}

void
SpatialLegendreFilter::updateFilter()
{
  if (!_use_displaced_mesh)
    return;

  const auto [min, max] = limitsFromMesh();
  if (max <= min)
    mooseError("The [Mesh] extent along the '" + std::string(getParam<MooseEnum>("axis")) +
               "' axis is degenerate (minimum = " + Moose::stringify(min) +
               ", maximum = " + Moose::stringify(max) + "), so the Legendre expansion in " +
               name() + " cannot be defined.");

  _min = min;
  _max = max;
  dynamic_cast<openmc::SpatialLegendreFilter *>(_filter)->set_minmax(_min, _max);
}

std::string
SpatialLegendreFilter::binName(unsigned int bin_index) const
{
  const std::string axis_name = _axis == 0 ? "x" : (_axis == 1 ? "y" : "z");
  return axis_name + "l" + Moose::stringify(bin_index);
}

#endif
