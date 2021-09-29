#pragma once

#include "NekUserObject.h"
#include "SpatialBinUserObject.h"

/**
 * Class that performs various postprocessing operations on the
 * NekRS solution with a spatial binning formed as the product
 * of an arbitrary number of combined single-set bins.
 */
class NekSpatialBinUserObject : public NekUserObject
{
public:
  static InputParameters validParams();

  NekSpatialBinUserObject(const InputParameters & parameters);

  virtual ~NekSpatialBinUserObject();

  virtual Real spatialValue(const Point & p) const override final;

  virtual const unsigned int bin(const Point & p) const;

  virtual const unsigned int num_bins() const;

  virtual const std::vector<Point> spatialPoints() const override { return _points; }

protected:
  /// Get the output points for a single bin
  void computePoints1D();

  /// Get the output points for two combined bins
  void computePoints2D();

  /// Get the output points for three combined bins
  void computePoints3D();

  /**
   * Get the coordinates for a point at the given indices for the bins
   * @param[in] indices indices of the bin distributions to combine
   * @param[out] p point at the (i, j, k) indices of the combined bins
   */
  void fillCoordinates(const std::vector<unsigned int> & indices, Point & p) const;

  /// Names of the userobjects providing the bins
  const std::vector<UserObjectName> & _bin_names;

  /// field to postprocess with the bins
  const field::NekFieldEnum _field;

  /**
   * Whether to map the NekRS space to bins by element centroid (false)
   * or quadrature point (true).
   */
  const bool & _map_space_by_qp;

  /// Userobjects providing the bins
  std::vector<const SpatialBinUserObject *> _bins;

  /// values of the userobject in each bin
  double * _bin_values;

  /// For each x, y, z direction, whether the combined distribution covers that direction
  std::vector<bool> _has_direction;

  /// For each x, y, z direction, which bin provides that direction
  std::vector<unsigned int> _bin_providing_direction;

  /// total number of bins
  unsigned int _n_bins;

  /// points at which to output the user object to give unique values
  std::vector<Point> _points;
};
