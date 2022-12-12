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

#include "NekSideIntegral.h"

registerMooseObject("CardinalApp", NekSideIntegral);

InputParameters
NekSideIntegral::validParams()
{
  InputParameters params = NekSideFieldPostprocessor::validParams();
  params.addClassDescription("Compute the integral of a field over a boundary of the NekRS mesh");
  return params;
}

NekSideIntegral::NekSideIntegral(const InputParameters & parameters)
  : NekSideFieldPostprocessor(parameters)
{
}

Real
NekSideIntegral::getValue()
{
  if (_field == field::velocity_component)
  {
    Real vx = nekrs::sideIntegral(_boundary, field::velocity_x, _pp_mesh);
    Real vy = nekrs::sideIntegral(_boundary, field::velocity_y, _pp_mesh);
    Real vz = nekrs::sideIntegral(_boundary, field::velocity_z, _pp_mesh);
    Point velocity(vx, vy, vz);
    return _velocity_direction * velocity;
  }

  return nekrs::sideIntegral(_boundary, _field, _pp_mesh);
}

#endif
