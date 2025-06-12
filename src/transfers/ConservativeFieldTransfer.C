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

#include "ConservativeFieldTransfer.h"
#include "NekInterface.h"

InputParameters
ConservativeFieldTransfer::validParams()
{
  auto params = FieldTransferBase::validParams();

  params.addRangeCheckedParam<Real>(
      "normalization_abs_tol",
      1e-8,
      "normalization_abs_tol > 0",
      "Absolute tolerance for checking if conservation is maintained during transfer");
  params.addRangeCheckedParam<Real>(
      "normalization_rel_tol",
      1e-5,
      "normalization_rel_tol > 0",
      "Relative tolerance for checking if conservation is maintained during transfer");
  params.addParam<std::string>(
      "postprocessor_to_conserve",
      "Name of the postprocessor/vectorpostprocessor containing the integral(s) used to ensure "
      "conservation; defaults to the name of the object plus '_integral'");

  params.addClassDescription("Base class for defining input parameters shared by conservative "
                             "transfers between NekRS and MOOSE");
  return params;
}

ConservativeFieldTransfer::ConservativeFieldTransfer(const InputParameters & parameters)
  : FieldTransferBase(parameters),
    _abs_tol(getParam<Real>("normalization_abs_tol")),
    _rel_tol(getParam<Real>("normalization_rel_tol"))
{
  nekrs::setAbsoluteTol(getParam<Real>("normalization_abs_tol"));
  nekrs::setRelativeTol(getParam<Real>("normalization_rel_tol"));

  if (isParamValid("postprocessor_to_conserve"))
    _postprocessor_name = getParam<std::string>("postprocessor_to_conserve");
  else
    _postprocessor_name = name() + "_integral";
}

std::string
ConservativeFieldTransfer::normalizationHint() const
{
  std::string s;
  s += "There are a few reason this might happen:\n\n"
       "- You forgot to add a transfer from another app to write into the " +
       _variable + "\n\n- You forgot to add a transfer from another app to write into the " +
       _postprocessor_name +
       "postprocessor, in which case the value of the postprocessor will always be zero.\n\n" +
       "- You have a mismatch between the NekRS mesh and the MOOSE mesh. Try visualizing the "
       "meshes in Paraview by running your input files with the --mesh-only flag.\n\n" +
       "- Your tolerances for comparing the re-normalized value with the incoming data are too "
       "tight. If the NekRS integrated value is close enough to the MOOSE integrated value, you "
       "can try relaxing the 'normalization_abs_tol' and/or 'normalization_rel_tol' parameters";
  return s;
}

#endif
