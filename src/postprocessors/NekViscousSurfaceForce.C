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
  auto nrs = nekrs::nrsPtr();
  auto mesh = nekrs::flowMesh();

  auto o_Sij   = nrs->strainRate();
  auto o_bID = platform->device.malloc<int>(_boundary.size(), _boundary.data());

  auto o_tangentialViscousTraction = nrs->viscousShearStress(o_bID, o_Sij); // tau dot n - ((tau dot n) dot n) * n
  auto o_normalViscousTraction = nrs->viscousNormalStress(o_bID, o_Sij); // ((tau dot n) dot n) * n

  const dlong Ntotal = o_tangentialViscousTraction.size() / mesh->dim;
  auto fvT = mesh->surfaceAreaMultiplyIntegrate(mesh->dim, Ntotal, o_bID, o_tangentialViscousTraction);
  auto fvN = mesh->surfaceAreaMultiplyIntegrate(mesh->dim, Ntotal, o_bID, o_normalViscousTraction);

  auto fx = fvT[0] + fvN[0];
  auto fy = fvT[1] + fvN[1];
  auto fz = fvT[2] + fvN[2];

  o_Sij.free();
  o_bID.free();

  if (_component == "total")
    return std::sqrt(fx*fx + fy*fy + fz*fz);
  else
    mooseError("x, y, and z components of viscous drag not currently supported. Please contact "
               "developers if this is affecting your analysis needs.");
}

#endif
