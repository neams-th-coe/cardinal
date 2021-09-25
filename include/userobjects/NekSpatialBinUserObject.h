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

protected:
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
};
