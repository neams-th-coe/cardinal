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

#ifdef ENABLE_NEK_COUPLING

#include "NekPressureSurfaceForce.h"

registerMooseObject("CardinalApp", NekPressureSurfaceForce);

InputParameters
NekPressureSurfaceForce::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addRequiredParam<Point>("direction", "Direction vector to dot with unit normal");
  params.addClassDescription("Compute pressure force on a surface along a particular direction");
  return params;
}

NekPressureSurfaceForce::NekPressureSurfaceForce(const InputParameters & parameters)
  : NekSidePostprocessor(parameters)
{
  auto d = getParam<Point>("direction");
  Point zero(0.0, 0.0, 0.0);
  if (d.absolute_fuzzy_equals(zero))
    mooseError("The 'direction' cannot have zero norm!");

  _direction = d / d.norm();
}

Real
NekPressureSurfaceForce::getValue()
{
  return nekrs::pressureSurfaceForce(_boundary, _direction);
}

#endif
