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

  /**
   * When using 'field = velocity_component', get the spatial value for a
   * particular component
   * @param[in] p point
   * @param[in] component component
   * @return value along direction of component
   */
  virtual Real spatialValue(const Point & p, const unsigned int & component) const = 0;

  virtual const unsigned int bin(const Point & p) const;

  virtual const unsigned int num_bins() const;

  virtual const std::vector<Point> spatialPoints() const override { return _points; }

  /// Compute the volume of each bin and check for zero contributions
  virtual void computeBinVolumes() final;

  /// Get the volume of each bin, used for normalizing in derived classes
  virtual void getBinVolumes() = 0;

  /**
   * Get the individual bin indices given a total combined bin
   * @param[in] total_bin_index total combined bin index
   * @return indices into each of the individual bin distributions
   */
  const std::vector<unsigned int> unrolledBin(const unsigned int & total_bin_index) const;

  /**
   * Get the integrating field
   * @return field
   */
  const field::NekFieldEnum & field() const { return _field; }

  /**
   * Get the point at which to evaluate the user object
   * @param[in] local_elem_id local element ID on the Nek rank
   * @param[in] local_node_id local node ID on the element
   * @return point, in dimensional form
   */
  Point nekPoint(const int & local_elem_id, const int & local_node_id) const;

protected:
  /// Get the output points for a single bin
  void computePoints1D();

  /// Get the output points for two combined bins
  void computePoints2D();

  /// Get the output points for three combined bins
  void computePoints3D();

  /// Reset the scratch space storage to zero values
  void resetPartialStorage();

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

  /**
   * Whether to throw an error if no GLL points or elements map to each bin
   * (which would indicate that the binning is probably way too fine relative
   * to the NekRS solution)
   */
  const bool & _check_zero_contributions;

  /// Userobjects providing the bins
  std::vector<const SpatialBinUserObject *> _bins;

  /// For each x, y, z direction, whether the combined distribution covers that direction
  std::vector<bool> _has_direction;

  /// For each x, y, z direction, which bin provides that direction
  std::vector<unsigned int> _bin_providing_direction;

  /**
   * Direction in which to evaluate velocity, if using 'field = velocity_component'.
   * Options: user (then provide a general vector direction with the 'velocity_direction' parameter
   *          normal (normal to the gap planes, only valid for side bin user objects)
   */
  component::BinnedVelocityComponentEnum _velocity_component;

  /// total number of bins
  unsigned int _n_bins;

  /// points at which to output the user object to give unique values
  std::vector<Point> _points;

  /// velocity direction to use for each bin
  std::vector<Point> _velocity_bin_directions;

  /// values of the userobject in each bin
  double * _bin_values;

  /// temporary storage space to hold the results of component-wise evaluations
  double * _bin_values_x;
  double * _bin_values_y;
  double * _bin_values_z;

  /// Volumes of each bin
  double * _bin_volumes;

  /**
   * Number of GLL points (for 'map_space_by_qp = true') or elements (for
   * 'map_space_by_qp = false') that contribute to each bin, for error checking
   */
  int * _bin_counts;

  /// Partial-sum of bin value per Nek rank
  double * _bin_partial_values;

  /// Partial-sum of bin count per Nek rank
  int * _bin_partial_counts;
};
