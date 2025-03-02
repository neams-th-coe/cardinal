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

#include "HeatTransferCoefficientAux.h"

registerMooseObject("CardinalApp", HeatTransferCoefficientAux);

InputParameters
HeatTransferCoefficientAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("heat_flux", "User object containing the wall-average heat flux");
  params.addRequiredParam<UserObjectName>("wall_T", "User object containing the wall-averaged temperature");
  params.addRequiredParam<UserObjectName>("bulk_T", "User object containing the averaged bulk temperature");
  params.addClassDescription(
      "Helper auxiliary kernel to compute a heat transfer coefficient given user objects for heat flux, wall temperature, and bulk temperature.");
  return params;
}

HeatTransferCoefficientAux::HeatTransferCoefficientAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _heat_flux(getUserObjectBase("heat_flux")),
    _wall_T(getUserObjectBase("wall_T")),
    _bulk_T(getUserObjectBase("bulk_T"))
{
}

Real
HeatTransferCoefficientAux::computeValue()
{
  Real q;
  Real Tw;
  Real Tinf;

  if (isNodal())
  {
    auto p = *_current_node;
    q = _heat_flux.spatialValue(p);
    Tw = _wall_T.spatialValue(p);
    Tinf = _bulk_T.spatialValue(p);
  }
  else
  {
    auto p = _current_elem->vertex_average();
    q = _heat_flux.spatialValue(p);
    Tw = _wall_T.spatialValue(p);
    Tinf = _bulk_T.spatialValue(p);
  }

  return q / (Tw - Tinf);
}
