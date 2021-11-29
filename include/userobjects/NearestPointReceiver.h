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

#include "GeneralUserObject.h"

#include "libmesh/point.h"

// Forward Declarations
class NearestPointReceiver;

template <>
InputParameters validParams<NearestPointReceiver>();

/**
 * Allows for setting values that are associated with points in space.
 * The spatialValue() function will then return the nearest value.
 */
class NearestPointReceiver : public GeneralUserObject
{
public:
  NearestPointReceiver(const InputParameters & parameters);
  virtual ~NearestPointReceiver();

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

  virtual Real spatialValue(const Point & p) const override;

  const std::vector<Point> & positions() { return _positions; }

  void setValues(const std::vector<Real> & values);

protected:
  /**
   * Find the position that is nearest to the point
   */
  unsigned int nearestPosition(const Point & p) const;

  const std::vector<Point> & _positions;

  std::vector<Real> _data;
};

