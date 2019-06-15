//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NEARESTPOINTRECEIVER_H
#define NEARESTPOINTRECEIVER_H

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

#endif // NEARESTPOINTRECEIVER_H
