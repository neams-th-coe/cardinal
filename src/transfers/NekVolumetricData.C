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

#include "NekVolumetricData.h"

registerMooseObject("CardinalApp", NekVolumetricData);

InputParameters
NekVolumetricData::validParams()
{
  auto params = FieldTransferBase::validParams();
  params.addClassDescription("Reads/writes volumetric source data between NekRS and MOOSE.");
  return params;
}

NekVolumetricData::NekVolumetricData(const InputParameters & parameters)
  : FieldTransferBase(parameters)
{
  if (_usrwrk_slot.size() > 1)
    paramError("usrwrk_slot",
               "'usrwrk_slot' must be of length 1 for general volume transfers; you have entered "
               "a vector of length " +
                   Moose::stringify(_usrwrk_slot.size()));

  addExternalVariable(_usrwrk_slot[0], _variable, 1.0, 1.0);

  if (!_nek_mesh->volume())
    mooseError("The NekVolumetricData object can only be used when there is volumetric coupling "
               "of NekRS with MOOSE, i.e. when 'volume = true' in NekRSMesh.");
}

void
NekVolumetricData::sendDataToNek()
{
  _console << "Sending volumetric data to NekRS..." << std::endl;

  auto d = 1.0;
  auto a = 0.0;
  for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); e++)
  {
    // We can only write into the nekRS scratch space if that element is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    _nek_problem.mapVolumeDataToNekVolume(e, _variable_number[_variable], d, a, &_v_elem);
    _nek_problem.writeVolumeSolution(_usrwrk_slot[0] * nekrs::fieldOffset(), e, _v_elem);
  }
}

#endif
