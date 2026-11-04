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

#include "NekDiffusiveFluxIntegral.h"

registerMooseObject("CardinalApp", NekDiffusiveFluxIntegral);

InputParameters
NekDiffusiveFluxIntegral::validParams()
{
  InputParameters params = NekSidePostprocessor::validParams();
  params.addParam<std::string>(
      "scalar",
      "Name of the scalar field in NekRS that this flux corresponds to. For example, if this "
      "transfer corresponds to a heat flux, then enter the name of the temperature scalar. If only "
      "one scalar is present, this defaults to that scalar.");
  params.addClassDescription(
      "Diffusive flux (e.g. a conductive heat flux) integrated over boundary(s) in the NekRS mesh");
  return params;
}

NekDiffusiveFluxIntegral::NekDiffusiveFluxIntegral(const InputParameters & parameters)
  : NekSidePostprocessor(parameters),
    _scalar_name(isParamValid("scalar") ? getParam<std::string>("scalar")
                                        : nekrs::onlyScalarName("NekDiffusiveFluxIntegral")),
    _scalar_id(nekrs::scalarID(_scalar_name))
{
}

Real
NekDiffusiveFluxIntegral::getValue() const
{
  return nekrs::diffusiveFluxIntegral(_boundary, _scalar_id, _pp_mesh);
}

#endif
