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

#include "NearestPointReceiver.h"

registerMooseObject("MooseApp", NearestPointReceiver);

InputParameters
NearestPointReceiver::validParams()
{
  auto params = GeneralUserObject::validParams();

  params.addRequiredParam<std::vector<Point>>("positions",
                                              "The positions the data will be associated with");

  params.addParam<std::vector<Real>>("default_data",
                                     {},
                                     "The default values of the data.  The number of entries must "
                                     "be the same as the number of positions");

  return params;
}

NearestPointReceiver::NearestPointReceiver(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _positions(getParam<std::vector<Point>>("positions")),
    _data(getParam<std::vector<Real>>("default_data"))
{
  if (_data.size() && (_data.size() != _positions.size()))
    paramError("default_data",
               "If default_data is specified then it should be the same length as the number of "
               "positions.");

  // Resize the data
  if (_data.empty())
    _data.resize(_positions.size());
}

NearestPointReceiver::~NearestPointReceiver() {}

void
NearestPointReceiver::execute()
{
}

Real
NearestPointReceiver::spatialValue(const Point & p) const
{
  auto nearest_pos = nearestPosition(p);

  return _data[nearest_pos];
}

void
NearestPointReceiver::setValues(const std::vector<Real> & values)
{
  if (values.size() != _data.size())
    mooseError("Setting values must be the same size as the number of points");

  _data = values;
}

unsigned int
NearestPointReceiver::nearestPosition(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for (auto i = beginIndex(_positions); i < _positions.size(); i++)
  {
    const auto & current_point = _positions[i];

    Real current_distance = (p - current_point).norm();

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = i;
    }
  }

  return closest;
}
