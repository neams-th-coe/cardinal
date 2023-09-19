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

#include "NekMassFluxWeightedSideIntegral.h"

registerMooseObject("CardinalApp", NekMassFluxWeightedSideIntegral);

InputParameters
NekMassFluxWeightedSideIntegral::validParams()
{
  InputParameters params = NekSideIntegral::validParams();
  params.addClassDescription(
      "Compute mass flux weighted integral of a field over a boundary of the NekRS mesh");
  return params;
}

NekMassFluxWeightedSideIntegral::NekMassFluxWeightedSideIntegral(const InputParameters & parameters)
  : NekSideIntegral(parameters)
{
  if (_field == field::velocity_component)
    mooseError("This class does not support 'field = velocity_component' because the "
               "velocity component normal to the sideset is used!");
}

Real
NekMassFluxWeightedSideIntegral::getValue() const
{
  return nekrs::sideMassFluxWeightedIntegral(_boundary, _field, _pp_mesh);
}

#endif
