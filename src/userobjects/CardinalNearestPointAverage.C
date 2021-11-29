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

#include "CardinalNearestPointAverage.h"

registerMooseObject("MooseApp", CardinalNearestPointAverage);

template <>
InputParameters
validParams<CardinalNearestPointAverage>()
{
  InputParameters params = nearestPointBaseValidParams<ElementAverageValue,
                                                       ElementVariableVectorPostprocessor>();

  params.addClassDescription(
      "Compute element variable integrals for nearest-point based subdomains");

  return params;
}

CardinalNearestPointAverage::CardinalNearestPointAverage(
    const InputParameters & parameters)
  : NearestPointBase<ElementAverageValue, ElementVariableVectorPostprocessor>(
        parameters),
    _np_post_processor_values(declareVector("np_post_processor_values"))
{
  _np_post_processor_values.resize(_user_objects.size());
}

Real
CardinalNearestPointAverage::spatialValue(const Point & point) const
{
  unsigned int i = nearestPointIndex(point);

  if (i >= _np_post_processor_values.size())
    mooseError("The vector length of vector post processor is ",
               _np_post_processor_values.size(),
               " but nearestPointIndex() return ",
               i);

  return _np_post_processor_values[i];
}

Real
CardinalNearestPointAverage::userObjectValue(unsigned int i) const
{
  if (i >= _np_post_processor_values.size())
    mooseError("The vector length of vector post processor is ",
               _np_post_processor_values.size(),
               " but you pass in ",
               i);

  return _np_post_processor_values[i];
}

void
CardinalNearestPointAverage::finalize()
{
  if (_user_objects.size() != _np_post_processor_values.size())
    mooseError("The vector length of the vector postproessor ",
               _np_post_processor_values.size(),
               " is different from the number of user objects ",
               _user_objects.size());

  unsigned int i = 0;
  for (auto & user_object : _user_objects)
  {
    user_object->finalize();
    _np_post_processor_values[i++] = user_object->getValue();
  }
}

unsigned int
CardinalNearestPointAverage::nearestPointIndex(const Point & p) const
{
  unsigned int closest = 0;
  Real closest_distance = std::numeric_limits<Real>::max();

  for (auto it : Moose::enumerate(_points))
  {
    const auto & current_point = it.value();

    Real current_distance = (p - current_point).norm();

    if (current_distance < closest_distance)
    {
      closest_distance = current_distance;
      closest = it.index();
    }
  }

  return closest;
}
