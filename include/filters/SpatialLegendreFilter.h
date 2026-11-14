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

#pragma once

#include "FilterBase.h"

#include "openmc/tallies/filter_sptl_legendre.h"

/**
 * A class which provides a thin wrapper around an OpenMC SpatialLegendreFilter
 * for use by Cardinal mapped tallies. This filter expands the tally score in
 * Legendre polynomials of the particle position along one of the Cartesian
 * axes. Unlike the spatial filters added by the tally objects, this filter does
 * not define an OpenMC -> MOOSE mapping; it simply subdivides each mapped tally
 * bin into a set of expansion coefficients, which can be evaluated on the mesh
 * with a SpatialLegendreAux.
 */
class SpatialLegendreFilter : public FilterBase
{
public:
  static InputParameters validParams();

  SpatialLegendreFilter(const InputParameters & parameters);

  /// Override to return false on the first bin and true on all other bins.
  virtual bool skipBin(const unsigned int bin) const override { return bin != 0; }

  /**
   * A function which returns the short-form name for each bin of
   * this filter. Used to label auxvariables a TallyBase scores in.
   * SpatialLegendreFilter(s) use the axis name followed by l.
   * @param[in] bin_index the bin index
   * @return a short name for the bin represented by bin_index
   */
  virtual std::string binName(unsigned int bin_index) const override;

  /**
   * Refresh the expansion limits. When 'track_mesh' is set, the limits are recomputed from
   * the current (possibly displaced) [Mesh] bounding box, so that the expansion follows a
   * domain which deforms during the solve. Otherwise this is a no-op and the user-specified
   * limits are retained.
   */
  virtual void updateFilter() override;

  /**
   * The order of the Legendre expansion.
   * @return the Legendre order
   */
  unsigned int getOrder() const { return _order; }

  /**
   * The Cartesian axis the expansion is applied to, where 0 is x, 1 is y, and 2 is z.
   * @return the component of the axis the expansion is applied along
   */
  unsigned int getAxis() const { return _axis; }

  /**
   * The minimum coordinate (in OpenMC's centimeter units) covered by the expansion.
   * @return the minimum coordinate of the expansion
   */
  Real getMinimum() const { return _min; }

  /**
   * The maximum coordinate (in OpenMC's centimeter units) covered by the expansion.
   * @return the maximum coordinate of the expansion
   */
  Real getMaximum() const { return _max; }

private:
  /// Compute the expansion limits from the current [Mesh] bounding box, in centimeters.
  std::pair<Real, Real> limitsFromMesh() const;

  /// The Legendre order.
  const unsigned int _order;

  /// The Cartesian axis the expansion is applied along (0 for x, 1 for y, 2 for z).
  const unsigned int _axis;

  /**
   * Whether the expansion limits are recomputed from the [Mesh] bounding box each time the
   * tallies are re-initialized, so that the expansion tracks a deforming domain.
   */
  const bool _track_mesh;

  /// The minimum coordinate along '_axis' covered by the expansion, in centimeters.
  Real _min;

  /// The maximum coordinate along '_axis' covered by the expansion, in centimeters.
  Real _max;
};
