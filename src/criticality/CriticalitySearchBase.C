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
#include "UserErrorChecking.h"
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
      "target", 1.0, "target > 0.0", "Target value of k effective to search for");
  params.addRequiredRangeCheckedParam<Real>(
      "root_tol", "root_tol > 0", "Absolute tolerance to converge root that yields target.");
  params.addRangeCheckedParam<Real>(
      "k_tol",
      1e-3,
      "k_tol > 0",
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
    _k_tol(getParam<Real>("k_tol")),
    _root_tol(getParam<Real>("root_tol")),
    _estimator(getParam<MooseEnum>("estimator").getEnum<eigenvalue::EigenvalueEnum>()),
    _target(getParam<Real>("target"))
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

  VariadicTable<int, Real, Real, Real> vt(
      {"Iteration", quantity() + " " + units(), "  k (mean)  ", " k (std dev) "});
  vt.setColumnFormat({VariadicTableColumnFormat::AUTO,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC,
                      VariadicTableColumnFormat::SCIENTIFIC});

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

    // fetch k and print values to console
    Real k = kMean(_estimator);
    Real k_std_dev = kStandardDeviation(_estimator);
    _k_values.push_back(k);
    _k_std_dev_values.push_back(k_std_dev);

    vt.addRow(_k_values.size() - 1, x, k, k_std_dev);
    vt.print(_console);

    if (_k_tol < 3 * k_std_dev)
      mooseDoOnce(mooseWarning(
          "The 'k_tol' for the criticality search (" + std::to_string(_k_tol) +
          ") is smaller than 3-sigma standard deviation in k (" + std::to_string(3 * k_std_dev) +
          "), which may require many search iterations to converge to this tolerance "
          "Consider a looser 'k_tol' or increase the number of particles."));

    return k - _target;
  };

  BrentsMethod::root(func, _minimum, _maximum, _root_tol);

  // check if the method converged
  if (abs(kMean(_estimator) - _target) >= _k_tol)
    mooseWarning("The eigenvalue produced by the criticality search was not within "
                 "specified k_tol of the target! This could occur if k_tol is too "
                 "tight for simulation's statistical error.");

  // fill the converged value into a postprocessor
  _openmc_problem->setPostprocessorValueByName(_pp_name, _inputs.back());
}

#endif
