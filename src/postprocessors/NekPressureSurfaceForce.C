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
  _direction = geom_utils::unitVector(getParam<Point>("direction"), "direction");

  if (_pp_mesh != nek_mesh::fluid)
    mooseError("The 'NekPressureSurfaceForce' postprocessor can only be applied to the fluid mesh boundaries!\n"
      "Please change 'mesh' to 'fluid'.");
}

Real
NekPressureSurfaceForce::getValue() const
{
  return nekrs::pressureSurfaceForce(_boundary, _direction, _pp_mesh);
}

#endif
