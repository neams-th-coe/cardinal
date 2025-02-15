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

#include "NekVolumeNorm.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekVolumeNorm);

InputParameters
NekVolumeNorm::validParams()
{
  InputParameters params = NekPostprocessor::validParams();
  params += NekFieldInterface::validParams();
  params.addRangeCheckedParam<unsigned int>("N", 2, "N>0", "L$^N$ norm to use");
  params.addClassDescription("Integrated L$^N$ norm of a NekRS solution field over the NekRS mesh");
  return params;
}

NekVolumeNorm::NekVolumeNorm(const InputParameters & parameters)
  : NekPostprocessor(parameters),
    NekFieldInterface(this, parameters),
    _N(getParam<unsigned int>("N"))
{
  if (_nek_problem->nondimensional())
    mooseError(
        "The NekVolumeNorm object does not yet support non-dimensional runs! Please contact the "
        "development team to accelerate this feature addition to support your use case.");
}

Real
NekVolumeNorm::getValue() const
{
  return nekrs::volumeNorm(_field, _pp_mesh, _function, _t, _N);
}

#endif
