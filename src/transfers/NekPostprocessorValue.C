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

#include "NekPostprocessorValue.h"

registerMooseObject("CardinalApp", NekPostprocessorValue);

InputParameters
NekPostprocessorValue::validParams()
{
  auto params = ScalarTransferBase::validParams();
  params.addClassDescription("Transfers a postprocessor value into NekRS");
  params.registerBase("ScalarTransfer");
  params.registerSystemAttributeName("ScalarTransfer");
  return params;
}

NekPostprocessorValue::NekPostprocessorValue(const InputParameters & parameters)
  : ScalarTransferBase(parameters)
{
  addExternalPostprocessor(name(), 0 /* initial value */);
  _postprocessor = &getPostprocessorValueByName(name());
}

void
NekPostprocessorValue::sendDataToNek()
{
  _console << "Sending postprocessor " << name() << " ("
           << Moose::stringify(*_postprocessor * _scaling) << ") to NekRS..." << std::endl;

  auto nrs = nekrs::nrsPtr();
  auto usrwrk = nekrs::host_wrk();
  usrwrk[_usrwrk_slot * nekrs::fieldOffset() + _offset] = *_postprocessor * _scaling;
}

#endif
