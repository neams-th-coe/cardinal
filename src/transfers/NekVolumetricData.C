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
  if (_transfer_mesh.contains("stl"))
    if (_direction == "to_nek")
      paramError("mesh", "Transferring data from the STL mesh to NekRS is not yet supported!");

  if (_usrwrk_slot.size() > 1)
    paramError("usrwrk_slot",
               "'usrwrk_slot' must be of length 1 for general volume transfers; you have entered "
               "a vector of length " +
                   Moose::stringify(_usrwrk_slot.size()));

  addExternalVariable(_usrwrk_slot[0], _variable, 0.0, 1.0);

  if (!_nek_mesh->volume())
    mooseError("The NekVolumetricData object can only be used when there is volumetric coupling "
               "of NekRS with MOOSE, i.e. when 'volume = true' in NekRSMesh.");

  for (const auto & t : _transfer_mesh)
    if (t == "stl")
      paramError("mesh",
                 "Transferring data from NekRS to the STL mesh is not yet supported for volumetric "
                 "data transfers!");
}

void
NekVolumetricData::sendDataToNek()
{
  _console << "Sending volumetric data to NekRS..." << std::endl;

  auto d = 1.0;
  auto a = 0.0;

  sendVolumetricDataToNek(_variable_number[_variable], d, a);
}

#endif
