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
  MooseEnum comp("x y z total", "total");
  params.addParam<MooseEnum>(
      "component",
      comp,
      "Component of pressure force to compute. 'total' takes the magnitude of the pressure force, "
      "while 'x', 'y', or 'z' return individual components.");
  params.addClassDescription("Compute pressure force that the fluid exerts on a surface");
  return params;
}

NekPressureSurfaceForce::NekPressureSurfaceForce(const InputParameters & parameters)
  : NekSidePostprocessor(parameters), _component(getParam<MooseEnum>("component"))
{
  if (_pp_mesh != nek_mesh::fluid)
    mooseError("The 'NekPressureSurfaceForce' postprocessor can only be applied to the fluid mesh boundaries!\n"
      "Please change 'mesh' to 'fluid'.");
}

Real
NekPressureSurfaceForce::getValue() const
{
  if (_component == "x")
    return nekrs::pressureSurfaceForce(_boundary, {1, 0, 0}, _pp_mesh);
  else if (_component == "y")
    return nekrs::pressureSurfaceForce(_boundary, {0, 1, 0}, _pp_mesh);
  else if (_component == "z")
    return nekrs::pressureSurfaceForce(_boundary, {0, 0, 1}, _pp_mesh);
  else if (_component == "total")
  {
    Real x = nekrs::pressureSurfaceForce(_boundary, {1, 0, 0}, _pp_mesh);
    Real y = nekrs::pressureSurfaceForce(_boundary, {0, 1, 0}, _pp_mesh);
    Real z = nekrs::pressureSurfaceForce(_boundary, {0, 0, 1}, _pp_mesh);
    return std::sqrt(x * x + y * y + z * z);
  }
  else
    mooseError("Unhandled component enum in NekPressureSurfaceForce!");
}

#endif
