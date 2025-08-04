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

#include "CriticalitySearchBase.h"
#include "VariadicTable.h"
#include "BrentsMethod.h"

InputParameters
CriticalitySearchBase::validParams()
{
  auto params = MooseObject::validParams();
  params += OpenMCBase::validParams();
  params.addRequiredParam<Real>(
      "minimum",
      "Minimum for values to search over; the root must occur at a value greater than the minimum");
  params.addRequiredParam<Real>(
      "maximum",
      "Maximum for values to search over; the root must occur at a value smaller than the maximum");
  params.addRangeCheckedParam<Real>(
      "tolerance",
      1e-3,
      "tolerance > 0",
      "Absolute tolerance to converge multiplication factor; be aware that if too few particles "
      "are used, statistical noise may require many criticality calculations to converge.");
  params.addParam<MooseEnum>(
      "estimator", getEigenvalueEnum(), "Type of eigenvalue estimator to use");
  params.addClassDescription(
      "Base class for defining parameters used in a criticality search in OpenMC.");
  params.registerBase("CriticalitySearch");
  params.registerSystemAttributeName("CriticalitySearch");
  params.addPrivateParam<OpenMCCellAverageProblem *>("_openmc_problem");
  return params;
}

CriticalitySearchBase::CriticalitySearchBase(const InputParameters & parameters)
  : MooseObject(parameters),
    OpenMCBase(this, parameters),
    _maximum(getParam<Real>("maximum")),
    _minimum(getParam<Real>("minimum")),
    _tolerance(getParam<Real>("tolerance")),
    _estimator(getParam<MooseEnum>("estimator").getEnum<eigenvalue::EigenvalueEnum>())
{
  if (_minimum >= _maximum)
    paramError("minimum",
               "The 'minimum' value (" + std::to_string(_minimum) +
                   ") must be less than the 'maximum' value (" + std::to_string(_maximum) + ").");

  auto pp_params = _factory.getValidParams("Receiver");
  _openmc_problem->addPostprocessor("Receiver", _pp_name, pp_params);
}

void
CriticalitySearchBase::searchForCriticality()
{
  _console << "Running criticality search in OpenMC for " << quantity() << " in range "
           << std::to_string(_minimum) << " - " << std::to_string(_maximum) << " " << units() << " "
           << std::endl;

  std::function<Real(Real)> func;
  func = [&](Real x)
  {
    // update the OpenMC model with a new parameter
    updateOpenMCModel(x);
    _inputs.push_back(x);

    // re-run the model
    int err = openmc_run();
    if (err)
      mooseError(openmc_err_msg);

    // fetch k and return the residual, k-1
    Real k = kMean(_estimator);
    Real k_std_dev = kStandardDeviation(_estimator);
    _k_values.push_back(k);
    _k_std_dev_values.push_back(k_std_dev);

    if (_tolerance < 3 * k_std_dev)
      mooseDoOnce(mooseWarning(
          "The 'tolerance' for the criticality search (" + std::to_string(_tolerance) +
          ") is smaller than 3-sigma standard deviation in k (" + std::to_string(3 * k_std_dev) +
          "); you may have to run a lot of criticality search points to converge to this "
          "tolerance. You may want to loosen 'tolerance' or increase the number of particles."));

    return k - 1.0;
  };

  BrentsMethod::root(func, _minimum, _maximum, _tolerance);

  // check if the method converged
  if (abs(kMean(_estimator) - 1.0) >= _tolerance)
    mooseError("Failed to converge criticality search! This may happen if your tolerance is too "
               "tight given the statistical error in the computation of k.");

  VariadicTable<int, Real, Real, Real> vt(
      {"Iteration", quantity() + " " + units(), "  k (mean)  ", " k (std dev) "});
  vt.setColumnFormat({VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC});
  for (int i = 0; i < _inputs.size(); ++i)
    vt.addRow(i, _inputs[i], _k_values[i], _k_std_dev_values[i]);
  vt.print(_console);

  // fill the converged value into a postprocessor
  _openmc_problem->setPostprocessorValueByName(_pp_name, _inputs.back());
}

#endif
