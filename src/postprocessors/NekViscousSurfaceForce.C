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

#include "NekViscousSurfaceForce.h"

registerMooseObject("CardinalApp", NekViscousSurfaceForce);

InputParameters
NekViscousSurfaceForce::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  MooseEnum comp("x y z total", "total");
  params.addParam<MooseEnum>(
      "component",
      comp,
      "Component of viscous force to compute. 'total' takes the magnitude of the viscous force, "
      "while 'x', 'y', or 'z' return individual components.");
  params.addClassDescription("Viscous force that the fluid exerts on a surface");
  return params;
}

NekViscousSurfaceForce::NekViscousSurfaceForce(const InputParameters & parameters)
  : NekSidePostprocessor(parameters), _component(getParam<MooseEnum>("component"))
{
  if (_pp_mesh != nek_mesh::fluid)
    mooseError("The 'NekViscousSurfaceForce' postprocessor can only be applied to the fluid mesh "
               "boundaries!\n"
               "Please change 'mesh' to 'fluid'.");

  if (_nek_problem->nondimensional())
    mooseError("The NekViscousSurfaceForce object is missing the implementation to convert the "
               "non-dimensional viscous drag to dimensional form. Please contact the developers if "
               "this is impacting your analysis.");
}

Real
NekViscousSurfaceForce::getValue() const
{
  auto drag = nekrs::viscousDrag(_boundary);

  if (_component == "total")
    return std::sqrt(drag[0] * drag[0] + drag[1] * drag[1] + drag[2] * drag[2]);
  else if (_component == "x")
    return drag[0];
  else if (_component == "y")
    return drag[1];
  else if (_component == "z")
    return drag[2];
  else
    mooseError("Unknown 'component' in NekViscousSurfaceForce!");
}

#endif
