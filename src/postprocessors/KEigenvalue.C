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

#include "KEigenvalue.h"

registerMooseObject("CardinalApp", KEigenvalue);

InputParameters
KEigenvalue::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("k eigenvalue computed by OpenMC");
  params.addParam<MooseEnum>("value_type",
                             getEigenvalueEnum(),
                             "Type of eigenvalue global tally to report");
  params.addParam<MooseEnum>(
      "output",
      getStatsOutputEnum(),
      "The value to output. Options are $k_{eff}$ (mean), the standard deviation "
      "of $k_{eff}$ (std_dev), or the relative error of $k_{eff}$ (rel_err).");
  return params;
}

KEigenvalue::KEigenvalue(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _type(getParam<MooseEnum>("value_type").getEnum<eigenvalue::EigenvalueEnum>()),
    _output(getParam<MooseEnum>("output").getEnum<statistics::OutputEnum>())
{
  if (openmc::settings::run_mode != openmc::RunMode::EIGENVALUE)
    mooseError("Eigenvalues are only computed when running OpenMC in eigenvalue mode!");
}

Real
KEigenvalue::getValue() const
{
  switch (_output)
  {
    case statistics::OutputEnum::Mean:
      return kMean(_type);

    case statistics::OutputEnum::StDev:
      return kStandardDeviation(_type);

    case statistics::OutputEnum::RelError:
      return kRelativeError();

    default:
      mooseError("Internal error: Unhandled statistics::OutputEnum enum in KEigenvalue.");
      break;
  }
}

Real
KEigenvalue::kRelativeError() const
{
  const auto mean = kMean(_type);
  return mean > 0.0 ? kStandardDeviation(_type) / mean : 0.0;
}

#endif
