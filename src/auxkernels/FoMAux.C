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

#ifdef ENABLE_OPENMC_COUPLING

#include "FoMAux.h"

registerMooseObject("CardinalApp", FoMAux);

InputParameters
FoMAux::validParams()
{
  auto params = OpenMCAuxKernel::validParams();
  params.addClassDescription("An auxkernel which computes a figure of merit for a tally.");
  params.addCoupledVar(
      "tally_value",
      "The variable containing the value of the tally. FoMAux "
      "assumes this is a volumetric quantity.");
  params.addCoupledVar(
      "tally_value_init",
      "The variable containing the tally value on the initial adaptivity step. FoMAux "
      "assumes this is a volumetric quantity. This is required for the AMR "
      "figure of merit.");
  params.addRequiredCoupledVar(
      "tally_rel_error",
      "The variable containing the statistical relative error of the tally. FoMAux "
      "assumes this is a volumetric quantity.");

  params.addRequiredParam<PostprocessorName>("sim_time", "The cumulative simulation time.");

  params.addRequiredParam<MooseEnum>(
      "fom_type",
      MooseEnum("VR AMR"),
      "The type of Figure of Merit (FoM) to compute. Options are the standard "
      "variance reduction FoM (VR) or the adaptive mesh refinement FoM (AMR).");

  return params;
}

FoMAux::FoMAux(const InputParameters & parameters)
  : OpenMCAuxKernel(parameters),
    _tally_val(isCoupled("tally_value") ? &coupledValue("tally_value") : nullptr),
    _tally_val_init(isCoupled("tally_value_init") ? &coupledValue("tally_value_init") : nullptr),
    _tally_val_rel_err(coupledValue("tally_rel_error")),
    _sim_time(getPostprocessorValue("sim_time")),
    _fom_type(getParam<MooseEnum>("fom_type").getEnum<FoMType>())
{
  if (_var.feType() != FEType(libMesh::CONSTANT, libMesh::MONOMIAL))
    paramError("variable",
               "FoMAux only supports CONSTANT MONOMIAL shape functions. Please "
               "ensure that 'variable' is of type MONOMIAL and order CONSTANT.");

  errorCoupledConstMonomial("tally_value");
  errorCoupledConstMonomial("tally_value_init");
  errorCoupledConstMonomial("tally_rel_error");

  // Error-check the different FoM options.
  switch (_fom_type)
  {
    case FoMType::VR:
      break;
    case FoMType::AMR:
      if (!_tally_val)
        paramError("tally_value",
                   "A tally value must be provided when using the AMR FoM.");
      if (!_tally_val_init)
        paramError("tally_value_init",
                   "An initial tally value must be provided when using the AMR FoM.");
      break;
    default:
    {
      mooseError("Unhandled FoMType enum in TallyFoMAux!");
      break;
    }
  }
}

Real
FoMAux::computeValue()
{
  switch (_fom_type)
  {
    case FoMType::VR:
      return 1.0 / _sim_time / _tally_val_rel_err[0] / _tally_val_rel_err[0];
    case FoMType::AMR:
      return std::abs((*_tally_val)[0] - (*_tally_val_init)[0]) / _sim_time / _tally_val_rel_err[0] / (*_tally_val_init)[0];
    default:
      return 0.0;
  }
}

#endif
